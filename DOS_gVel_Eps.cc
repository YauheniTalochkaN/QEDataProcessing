#include <iostream>
#include <cmath>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <filesystem>
#include <fstream>
#include <algorithm>

#include "Constants.hh"

#include "pugixml.hpp"
#include <yaml-cpp/yaml.h>

#include "torch/torch.h"
#include "c10/cuda/CUDACachingAllocator.h"

#include <TROOT.h>
#include <TVector3.h>
#include "TMatrixD.h"
#include "TApplication.h"
#include "TRootCanvas.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/draw_triangulation_3.h>
#include <CGAL/Search_traits_3.h>
#include <CGAL/Kd_tree.h>
#include <CGAL/K_neighbor_search.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel epic_kernel_CGAL;
typedef CGAL::Triangulation_vertex_base_with_info_3<size_t, epic_kernel_CGAL> tvb_info_CGAL;
typedef CGAL::Triangulation_data_structure_3<tvb_info_CGAL> tds_CGAL;
typedef CGAL::Delaunay_triangulation_3<epic_kernel_CGAL, tds_CGAL> delaunay_CGAL;
typedef delaunay_CGAL::Point point_CGAL;
typedef CGAL::Search_traits_3<epic_kernel_CGAL> st_CGAL;
typedef CGAL::Kd_tree<st_CGAL> kdt_CGAL;
typedef CGAL::K_neighbor_search<st_CGAL> kns_CGAL;

using PointGenerator = std::function<std::vector<TVector3>(const TVector3&)>;

struct TVector3Hash 
{
    inline std::size_t operator()(const TVector3& v) const 
    {
        auto quantize = [](double x) -> double 
                        {
                            return std::trunc(x / Constants::k_eps) * Constants::k_eps;
                        };

        std::size_t seed = 0;

        auto hash_combine = [&seed](double val) 
        {
            std::hash<double> hasher;
            
            seed ^= hasher(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        };

        hash_combine(quantize(v.X()));
        hash_combine(quantize(v.Y()));
        hash_combine(quantize(v.Z()));

        return seed;
    }
};

struct TVector3Equal 
{
    inline bool operator()(const TVector3& lhs, const TVector3& rhs) const 
    {
        return std::abs(lhs.X() - rhs.X()) < Constants::k_eps &&
               std::abs(lhs.Y() - rhs.Y()) < Constants::k_eps &&
               std::abs(lhs.Z() - rhs.Z()) < Constants::k_eps;
    }
};

int main(int argc, char* argv[])
{    
    std::string yamlName;
    
    if(argc < 2)
	{
    	yamlName = "../configs/params.yaml";
    }
    else
    {
        yamlName = argv[1];
    }
    
    std::filesystem::path yamlFilePath(yamlName);

    if(!std::filesystem::exists(yamlFilePath)) 
    {
        throw std::invalid_argument("Required YAML file was not found.");
    }
    
    YAML::Node yaml_node;
        
    try 
    {
        yaml_node = YAML::LoadFile(yamlFilePath.string());
    }
    catch(const YAML::BadFile& ex) 
    {
        throw std::runtime_error("Failed to open or read YAML file " + yamlFilePath.string() + ": " + ex.what());
    } 
    catch(const YAML::ParserException& ex) 
    {
        throw std::runtime_error("Failed to parse YAML file " + yamlFilePath.string() + ": " + ex.what());
    }
    
    std::string data_dir = yaml_node["data_dir"].as<std::string>();
    std::string data_schema_file = yaml_node["data_schema_file"].as<std::string>();
    bool use_inv = yaml_node["use_inv"].as<bool>();
    double Eg_exp = yaml_node["Eg_exp"].as<double>();
    double gsp = yaml_node["gsp"].as<double>();
    double En_step = yaml_node["En_step"].as<double>();
    double smearing = yaml_node["smearing"].as<double>();
    int32_t num_k_neighbors = yaml_node["num_k_neighbors"].as<int32_t>();
    std::string torch_device = yaml_node["torch_device"].as<std::string>();

    torch::Device device = torch::kCPU;

    if(torch_device == "CUDA")
    {
        device = torch::kCUDA;
    }

    auto parseDoubleLine = [](std::string str) -> std::vector<double>
    {        
        std::replace(str.begin(), str.end(), ',', ' ');
        std::replace(str.begin(), str.end(), ';', ' ');
        
        std::istringstream iss(str);
        std::vector<double> result;
        
        double val;
        
        while(iss >> val) 
        {
            result.push_back(val);
        }

        return result;
    };

    std::filesystem::path data_schema_path = std::filesystem::path(data_dir) / data_schema_file;

    pugi::xml_document xml_doc;

    pugi::xml_parse_result result = xml_doc.load_file(data_schema_path.c_str());

    if(!result) 
    {
        std::cerr << "Failed to parse XML file " << data_schema_path
                  << ": " << result.description() << std::endl;
        std::exit(1);
    }

    int64_t last_occupied = 0L;
    std::vector<TVector3> irreducible_kvec_list;
    std::vector<std::vector<double>> irreducible_energy_list;

    for(pugi::xpath_node ks_node_path : xml_doc.select_nodes("//ks_energies")) 
    {
        pugi::xml_node ks_node = ks_node_path.node();

        for(pugi::xml_node kp_node : ks_node.children("k_point")) 
        {
            std::vector<double> vals = parseDoubleLine(kp_node.child_value());
            
            irreducible_kvec_list.emplace_back(vals.at(0), vals.at(1), vals.at(2));
        }

        for(pugi::xml_node ev_node : ks_node.children("eigenvalues")) 
        {
            std::vector<double> vals = parseDoubleLine(ev_node.child_value());

            std::transform(vals.begin(), vals.end(), vals.begin(), 
                          [](double x) -> double {return x * 2.0 * Constants::Ry;});

            irreducible_energy_list.push_back(vals);
        }

        for(pugi::xml_node occv_node : ks_node.children("occupations")) 
        {
            std::vector<double> vals = parseDoubleLine(occv_node.child_value());

            auto it = std::find_if(vals.begin(), vals.end(),
                                   [](double x) {return std::abs(x) < std::numeric_limits<double>::epsilon();});
            
            if(it != vals.end()) 
            {
                int64_t idx = std::distance(vals.begin(), it) - 1UL;
                
                last_occupied = std::max(last_occupied, idx);
            }
        }
    }

    const size_t num_irreducible_kvecs = irreducible_kvec_list.size();
    const int64_t num_branches = irreducible_energy_list[0].size();
    
    double alat = Constants::aB * xml_doc.select_node("//atomic_structure").node().attribute("alat").as_double();
    
    pugi::xml_node reciprocal_lattice_node = xml_doc.select_node("//reciprocal_lattice").node();

    TVector3 b1, b2, b3;
    
    pugi::xml_node b1_node = reciprocal_lattice_node.child("b1");

    if(b1_node) 
    {
        std::vector<double> vals = parseDoubleLine(b1_node.child_value());
        
        b1.SetXYZ(vals.at(0), vals.at(1), vals.at(2));
    }

    pugi::xml_node b2_node = reciprocal_lattice_node.child("b2");

    if(b2_node) 
    {
        std::vector<double> vals = parseDoubleLine(b2_node.child_value());
        
        b2.SetXYZ(vals.at(0), vals.at(1), vals.at(2));
    }

    pugi::xml_node b3_node = reciprocal_lattice_node.child("b3");

    if(b3_node) 
    {
        std::vector<double> vals = parseDoubleLine(b3_node.child_value());
        
        b3.SetXYZ(vals.at(0), vals.at(1), vals.at(2));
    }

    TMatrixD Mb(3, 3);

    Mb[0][0] = b1.X(); Mb[0][1] = b2.X(); Mb[0][2] = b3.X();
    Mb[1][0] = b1.Y(); Mb[1][1] = b2.Y(); Mb[1][2] = b3.Y();
    Mb[2][0] = b1.Z(); Mb[2][1] = b2.Z(); Mb[2][2] = b3.Z();

    TMatrixD Mb_inv = Mb;
    Mb_inv.Invert();

    pugi::xml_node symmetries_node = xml_doc.select_node("//symmetries").node();

    std::vector<TMatrixD> group_matrices;

    for(pugi::xml_node symmetry_node : symmetries_node.children("symmetry")) 
    {
        pugi::xml_node rotation_node = symmetry_node.child("rotation");
        
        std::vector<double> vals1 = parseDoubleLine(rotation_node.child_value());

        TMatrixD G(3, 3);

        G[0][0] = vals1[0]; G[0][1] = vals1[1]; G[0][2] = vals1[2];
        G[1][0] = vals1[3]; G[1][1] = vals1[4]; G[1][2] = vals1[5];
        G[2][0] = vals1[6]; G[2][1] = vals1[7]; G[2][2] = vals1[8];

        group_matrices.push_back(Mb * G.Invert().T() * Mb_inv);
    }   

    PointGenerator group_generator = [&group_matrices, &Mb, &Mb_inv](const TVector3& vec) -> std::vector<TVector3>
    {
        std::vector<TVector3> result;

        auto wrap_frac = [](double x) -> double 
        {
            double w = std::fmod(x, 1.0);
            
            if (w > 0.5)  w -= 1.0;
            if (w < -0.5) w += 1.0;
            
            return w;
        };

        for(const auto& g : group_matrices)
        {
            TVector3 k_c = g * vec;

            TVector3 k_b = Mb_inv * k_c;

            k_b.SetX(wrap_frac(k_b.X()));
            k_b.SetY(wrap_frac(k_b.Y()));
            k_b.SetZ(wrap_frac(k_b.Z()));

            result.push_back(Mb * k_b);
        }

        return result;
    };

    torch::NoGradGuard no_grad;

    std::vector<torch::Tensor> wfc_ten_list(num_irreducible_kvecs);
    std::vector<torch::Tensor> Gk_ten_list(num_irreducible_kvecs);

    for(size_t i = 0; i < num_irreducible_kvecs; ++i) 
    {
        std::filesystem::path filename = std::filesystem::path(data_dir) / ("wfc" + std::to_string(i + 1) + ".dat");

        std::ifstream wfc_file(filename, std::ios::binary);

        if(!wfc_file)
        {
            std::cerr << "Failed to open XML file " << filename << std::endl;
            std::exit(1);
        }

        wfc_file.seekg(60, std::ios::cur);

        int32_t igwx;
        int32_t npol;

        wfc_file.read(reinterpret_cast<char*>(&igwx), 4);
        wfc_file.read(reinterpret_cast<char*>(&npol), 4);

        wfc_file.seekg(92, std::ios::cur);

        torch::Tensor Gk_ten = torch::empty({igwx, 3}, 
                                            torch::TensorOptions().dtype(torch::kComplexDouble));

        for(int32_t j = 0; j < igwx; ++j)
        {
            std::array<int32_t, 3> mill;
            
            wfc_file.read(reinterpret_cast<char*>(mill.data()), 3 * sizeof(int32_t));

            TVector3 mill_vec(mill[0], mill[1], mill[2]);
            TVector3 Gk_vec = Mb * mill_vec + irreducible_kvec_list[i];

            Gk_ten[j] = torch::tensor({Gk_vec.X(), Gk_vec.Y(), Gk_vec.Z()}, 
                                      torch::TensorOptions().dtype(torch::kComplexDouble));
        }

        Gk_ten_list[i] = Gk_ten.to(device);      

        torch::Tensor coeffs_ten = torch::empty({num_branches, npol, igwx},
                                                torch::TensorOptions().dtype(torch::kComplexDouble));

        for(int64_t j = 0; j < num_branches; ++j) 
        {
            wfc_file.seekg(8, std::ios::cur);

            std::vector<std::complex<double>> coeffs(npol * igwx);

            wfc_file.read(reinterpret_cast<char*>(coeffs.data()), npol * igwx * sizeof(std::complex<double>));

            coeffs_ten[j] = torch::from_blob(coeffs.data(), {npol, igwx},
                                             torch::TensorOptions().dtype(torch::kComplexDouble));
        }

        wfc_ten_list[i] = coeffs_ten.to(device);

        wfc_file.close();
    }

    std::cout << "Constructing triangulation..." << std::endl;

    std::vector<torch::Tensor> kvec_ten_list;
    std::vector<torch::Tensor> energy_ten_list;

    std::vector<point_CGAL> kpoint_b_cgal_list;
    std::vector<point_CGAL> kpoint_cart_cgal_list;

    std::unordered_set<TVector3, TVector3Hash, TVector3Equal> nodes;

    constexpr auto is_border = [](double b) -> bool
    {
        return (std::abs(b) <= (0.5 + 10.0 * Constants::k_eps)) && (std::abs(b) >= (0.5 - 10.0 * Constants::k_eps));
    };

    auto add_translations_if_border = [&b1, &b2, &b3, &Mb_inv, is_border](std::vector<TVector3>& list)
    {      
        size_t n = list.size();

        for(size_t i = 0; i < n; ++i)
        {
            const TVector3& vec = list[i];
            
            TVector3 vec_b = Mb_inv * vec;
            
            if(is_border(vec_b.X()) || is_border(vec_b.Y()) || is_border(vec_b.Z()))
            {
                for(double i1 = -1.0; i1 <= 1.0; ++i1)
                {
                    for(double i2 = -1.0; i2 <= 1.0; ++i2)
                    {
                        for(double i3 = -1.0; i3 <= 1.0; ++i3)
                        {
                            TVector3 tvec = vec + i1 * b1 + i2 * b2 + i3 * b3;

                            TVector3 tvec_b = Mb_inv * tvec;

                            if((std::abs(tvec_b.X()) <= (0.5 + 10.0 * Constants::k_eps)) &&
                               (std::abs(tvec_b.Y()) <= (0.5 + 10.0 * Constants::k_eps)) &&
                               (std::abs(tvec_b.Z()) <= (0.5 + 10.0 * Constants::k_eps)))
                            {
                                list.push_back(tvec);   
                            }
                        }
                    }
                }
            }
        }
    };

    std::vector<int64_t> rkvec_to_irrkvec;

    for(size_t i = 0; i < num_irreducible_kvecs; ++i)
    {        
        std::vector<TVector3> all_kvecs = group_generator(irreducible_kvec_list[i]);

        add_translations_if_border(all_kvecs);

        for(const auto& vec : all_kvecs)
        {
            if(nodes.find(vec) == nodes.end()) 
            {
                nodes.insert(vec);

                TVector3 vec_b = Mb_inv * vec;

                kpoint_b_cgal_list.emplace_back(std::round(vec_b.X() / Constants::k_eps) * Constants::k_eps, 
                                                std::round(vec_b.Y() / Constants::k_eps) * Constants::k_eps, 
                                                std::round(vec_b.Z() / Constants::k_eps) * Constants::k_eps);

                kpoint_cart_cgal_list.emplace_back(vec.X(), vec.Y(), vec.Z());

                rkvec_to_irrkvec.push_back(i);

                kvec_ten_list.push_back(torch::tensor({vec.X(), vec.Y(), vec.Z()}, 
                                                      torch::TensorOptions().dtype(torch::kFloat64)));
                
                energy_ten_list.push_back(torch::tensor(irreducible_energy_list[i], 
                                                        torch::TensorOptions().dtype(torch::kFloat64)));
            }
        }
    }

    nodes.clear();

    torch::Tensor kTen = torch::stack(kvec_ten_list, 0).to(device);
    torch::Tensor energyTen = torch::stack(energy_ten_list, 0).to(device);

    torch::Tensor Et_vb = energyTen.select(1, last_occupied);
    torch::Tensor Eb_cb = energyTen.select(1, last_occupied + 1);

    int64_t id_min = torch::argmin(Eb_cb - Et_vb).item<int64_t>();
    int64_t id_max = torch::argmax(Et_vb).item<int64_t>();

    double Et_vb_val = Et_vb[id_max].item<double>();
    double Eop_vb_val = Et_vb[id_min].item<double>();
    double Eop_cb_val = Eb_cb[id_min].item<double>();
    
    torch::Tensor Eshift = torch::empty({num_branches}, energyTen.options());
    Eshift.slice(0, 0, last_occupied + 1).fill_(Et_vb_val);
    Eshift.slice(0, last_occupied + 1).fill_(Eop_cb_val + Et_vb_val - Eop_vb_val - Eg_exp);

    energyTen = energyTen - Eshift;

    kvec_ten_list.clear();
    energy_ten_list.clear();

    std::vector<size_t> kinfo_list(kpoint_b_cgal_list.size());
    std::iota(kinfo_list.begin(), kinfo_list.end(), 0);

    delaunay_CGAL triangulation(boost::make_zip_iterator(boost::make_tuple(kpoint_b_cgal_list.begin(), kinfo_list.begin())),
                                boost::make_zip_iterator(boost::make_tuple(kpoint_b_cgal_list.end(), kinfo_list.end())));

    std::vector<std::vector<int64_t>> tetrahedron_list;
    std::vector<double> tetrahedron_volume_list;

    double total_volume = 0.0;

    for(auto cit = triangulation.finite_cells_begin(); cit != triangulation.finite_cells_end(); ++cit) 
    {        
        std::vector<int64_t> id_list(4);
        std::vector<point_CGAL> points(4);

        bool take = true;
        
        for(int i = 0; i < 4; ++i) 
        {
            size_t id = cit->vertex(i)->info();

            id_list[i] = id;

            point_CGAL kpoint_b = cit->vertex(i)->point();

            if((kpoint_b.x() < -Constants::k_eps) && use_inv)
            {
                take = false;   
            }

            cit->vertex(i)->set_point(kpoint_cart_cgal_list[id]);

            points[i] = cit->vertex(i)->point();
        }

        if(take)
        {
            tetrahedron_list.push_back(id_list);

            double vol = CGAL::volume(points[0], points[1], points[2], points[3]);

            tetrahedron_volume_list.push_back(vol);

            total_volume += vol;
        }
    }

    kpoint_b_cgal_list.clear();
    kinfo_list.clear();

    std::cout << "Triangulation was completed." << std::endl;

    CGAL::draw(triangulation);

    std::cout << "Calculating M_ij(n,m,k) = <m,k|∇_i|n,k> ∙ <n,k|∇_j|m,k>, where n ∈ VB, m ∈ CB ..." << std::endl;

    std::vector<torch::Tensor> Mij_ten_list(num_irreducible_kvecs);
    std::vector<torch::Tensor> hw_ten_list(num_irreducible_kvecs);

    double hwmax = 0.0;

    /* std::ofstream Wnm_file("Wnm.csv");

    if(!Wnm_file.is_open()) 
    {
        std::cerr << "Error when creating Wnm.csv file." << std::endl;

        return 1;
    }

    Wnm_file << "energy_vb(eV),energy_cb(eV),amp(Å^-2 eV^-2)\n"; */

    for(size_t i = 0; i < num_irreducible_kvecs; ++i)
    {
        torch::Tensor wfc_vb = wfc_ten_list[i].slice(0, 0, last_occupied + 1);
        torch::Tensor wfc_cb = wfc_ten_list[i].slice(0, last_occupied + 1);
        
        torch::Tensor amp = (2.0 * M_PI / alat) * torch::einsum("nsg,msg,gd->nmd", 
                                                                {wfc_vb, wfc_cb.conj(), Gk_ten_list[i]});

        Mij_ten_list[i] = torch::einsum("...i,...j->...ij", {amp, amp.conj()}).reshape({-1, 3, 3});

        torch::Tensor Energy = torch::tensor(irreducible_energy_list[i], 
                                             torch::TensorOptions().dtype(torch::kFloat64)).to(device);

        Energy = Energy - Eshift;

        torch::Tensor Energy_vb = Energy.slice(0, 0, last_occupied + 1);
        torch::Tensor Energy_cb = Energy.slice(0, last_occupied + 1);

        hw_ten_list[i] = (Energy_cb.unsqueeze(0) - Energy_vb.unsqueeze(1)).reshape({-1});

        hwmax = std::max({hwmax, torch::amax(hw_ten_list[i]).item<double>()});

        /* torch::Tensor dEnm2 = (Energy_cb.unsqueeze(0) - Energy_vb.unsqueeze(1)).pow_(2.0);
        torch::Tensor Wnm = amp.norm(2, 2).pow_(2.0) / dEnm2;

        int64_t num_vb = Wnm.size(0);
        int64_t num_cb = Wnm.size(1);

        torch::Tensor Wnm_cpu = Wnm.cpu();
        torch::Tensor Energy_vb_cpu = Energy_vb.cpu();
        torch::Tensor Energy_cb_cpu = Energy_cb.cpu();

        double* Wnm_ptr = Wnm_cpu.data_ptr<double>();
        double* Energy_vb_ptr = Energy_vb_cpu.data_ptr<double>();
        double* Energy_cb_ptr = Energy_cb_cpu.data_ptr<double>();

        for(int64_t n = 0; n < num_vb; ++n) 
        {
            for(int64_t m = 0; m < num_cb; ++m) 
            {
                Wnm_file << Energy_vb_ptr[n] << "," << Energy_cb_ptr[m] << "," << Wnm_ptr[n * num_cb + m] << "\n";
            }
        } */
    }

    //Wnm_file.close();

    std::cout << "M_ij(n,m,k) were calculated." << std::endl;

    std::cout << "Calculating group velocity vectors in the Brillouin zone..." << std::endl;

    const size_t num_fb_kvec = kpoint_cart_cgal_list.size();

    std::unordered_map<TVector3, std::pair<torch::Tensor, int64_t>, TVector3Hash, TVector3Equal> point_to_index_map;

    for(size_t i = 0; i < num_fb_kvec; ++i)
    {
        TVector3 vec(kpoint_cart_cgal_list[i].x(), 
                     kpoint_cart_cgal_list[i].y(), 
                     kpoint_cart_cgal_list[i].z());
        
        point_to_index_map[vec] = {torch::tensor({0.0, 0.0, 0.0}, 
                                   torch::TensorOptions().dtype(torch::kFloat64)), i};

        TVector3 vec_b = Mb_inv * vec;

        for(double i1 = -1.0; i1 <= 1.0; ++i1)
        {
            for(double i2 = -1.0; i2 <= 1.0; ++i2)
            {
                for(double i3 = -1.0; i3 <= 1.0; ++i3)
                {
                    if(!((i1 == 0.0) && (i2 == 0.0) && (i3 == 0.0)) && 
                       !(((std::abs(i1) == 1.0) && is_border(vec_b.X())) ||
                         ((std::abs(i2) == 1.0) && is_border(vec_b.Y())) ||
                         ((std::abs(i3) == 1.0) && is_border(vec_b.Z()))))
                    {
                        TVector3 tr = i1 * b1 + i2 * b2 + i3 * b3;
                        TVector3 tvec = vec + tr;

                        kpoint_cart_cgal_list.emplace_back(tvec.X(), tvec.Y(), tvec.Z());

                        point_to_index_map[tvec] = {torch::tensor({tr.X(), tr.Y(), tr.Z()}, 
                                                    torch::TensorOptions().dtype(torch::kFloat64)), i};  
                    }
                }
            }
        }
    }

    kdt_CGAL tree(kpoint_cart_cgal_list.begin(), kpoint_cart_cgal_list.end());

    std::vector<std::vector<torch::Tensor>> shifts(num_fb_kvec);
    std::vector<std::vector<int64_t>> neighbors(num_fb_kvec);

    for(size_t i = 0; i < num_fb_kvec; ++i)
    {
        kns_CGAL search(tree, kpoint_cart_cgal_list[i], num_k_neighbors + 1);

        for(auto it = search.begin(); it != search.end(); ++it)
        {
            TVector3 vec(it->first.x(), it->first.y(), it->first.z());

            auto [ten, id] = point_to_index_map[vec];
            
            shifts[i].push_back(ten);
            neighbors[i].push_back(id);
        }
    }

    kpoint_cart_cgal_list.clear();
    point_to_index_map.clear();
    tree.clear();

    std::vector<torch::Tensor> gvel_ten_list;

    for(size_t i = 0; i < num_fb_kvec; ++i)
    {       
        torch::Tensor index_tensor = torch::tensor(neighbors[i], torch::dtype(torch::kLong)).to(device);
        torch::Tensor shift = torch::stack(shifts[i], 0).to(device);
        
        torch::Tensor dhk = 2.0 * M_PI * Constants::hbar / (alat * 1.0E-8) * (kTen.index_select(0, index_tensor) + shift - kTen[i]);

        torch::Tensor dhkx = dhk.select(1, 0);
        torch::Tensor dhky = dhk.select(1, 1);
        torch::Tensor dhkz = dhk.select(1, 2);

        torch::Tensor ones = torch::ones_like(dhkx);

        double coeff = std::pow(Constants::c, 2.0) / (2.0 * Constants::me); 

        torch::Tensor pxx = coeff * dhkx * dhkx;
        torch::Tensor pyy = coeff * dhky * dhky;
        torch::Tensor pzz = coeff * dhkz * dhkz;
        torch::Tensor pxy = coeff * dhkx * dhky;
        torch::Tensor pxz = coeff * dhkx * dhkz;
        torch::Tensor pyz = coeff * dhky * dhkz;

        torch::Tensor P = torch::cat({ones.unsqueeze(1),   
                                      dhkx.unsqueeze(1), dhky.unsqueeze(1), dhkz.unsqueeze(1),     
                                      pxx.unsqueeze(1),  pyy.unsqueeze(1),  pzz.unsqueeze(1),
                                      pxy.unsqueeze(1),  pxz.unsqueeze(1),  pyz.unsqueeze(1)}, 1);
  
        torch::Tensor P_pinv = torch::linalg_pinv(P);

        torch::Tensor En_target = energyTen.index_select(0, index_tensor);

        torch::Tensor params = torch::matmul(P_pinv, En_target).t();

        torch::Tensor abs_vel = params.slice(1, 1, 4).norm(2, 1);

        abs_vel.masked_fill_(abs_vel < 1.0, 0.0);

        gvel_ten_list.push_back(abs_vel);
    }

    torch::Tensor gvelTen = torch::stack(gvel_ten_list, 0);

    std::cout << "Group velocity vectors were calculated." << std::endl;
                                        
    std::cout << "Calculating the density of states and the average magnitude of group velocity..." << std::endl;

    auto start = std::chrono::steady_clock::now();

    double Emin = torch::amin(energyTen).item<double>();
    double Emax = torch::amax(energyTen).item<double>();

    torch::Tensor Emesh = torch::linspace(Emin, Emax, static_cast<int64_t>(std::trunc((Emax - Emin) / En_step)) + 1L, 
                                          torch::TensorOptions().dtype(torch::kFloat64)).to(device);

    torch::Tensor Emesh_ext = Emesh.unsqueeze(-1);

    const int64_t num_enpoints = Emesh.size(0);

    torch::Tensor dos = torch::zeros({num_enpoints}, 
                                     torch::TensorOptions().dtype(torch::kFloat64)).to(device);

    torch::Tensor avgVel = torch::zeros({num_enpoints}, 
                                        torch::TensorOptions().dtype(torch::kFloat64)).to(device);

    torch::Tensor pdos = torch::zeros({num_enpoints, num_branches}, 
                                      torch::TensorOptions().dtype(torch::kFloat64)).to(device);

    torch::Tensor pvel = torch::zeros({num_enpoints, num_branches}, 
                                      torch::TensorOptions().dtype(torch::kFloat64)).to(device);

    for(size_t i = 0; i < tetrahedron_list.size(); ++i)
    {        
        std::cout << "Tetrahedra: " << i + 1UL << "/" << tetrahedron_list.size() << "\r" << std::flush;
        
        torch::Tensor index_tensor = torch::tensor(tetrahedron_list[i], torch::dtype(torch::kLong)).to(device);

        torch::Tensor tetEnergyTen = energyTen.index_select(0, index_tensor);
        torch::Tensor tetgVelTen = gvelTen.index_select(0, index_tensor);

        auto [tetEnergyTen_sort, sortIDs] = torch::sort(tetEnergyTen, 0);
        torch::Tensor tetgVelTen_sort = torch::gather(tetgVelTen, 0, sortIDs);

        torch::Tensor e1 = tetEnergyTen_sort[0].unsqueeze(0) - 2.0 * Constants::en_eps;
        torch::Tensor e2 = tetEnergyTen_sort[1].unsqueeze(0) - Constants::en_eps;
        torch::Tensor e3 = tetEnergyTen_sort[2].unsqueeze(0) + Constants::en_eps;
        torch::Tensor e4 = tetEnergyTen_sort[3].unsqueeze(0) + 2.0 * Constants::en_eps;
        
        torch::Tensor e21 = e2 - e1;
        torch::Tensor e31 = e3 - e1;
        torch::Tensor e41 = e4 - e1;
        torch::Tensor e32 = e3 - e2;
        torch::Tensor e42 = e4 - e2;
        torch::Tensor e43 = e4 - e3;
        torch::Tensor em1 = Emesh_ext - e1;
        torch::Tensor em2 = Emesh_ext - e2;
        torch::Tensor em3 = Emesh_ext - e3;
        torch::Tensor em4 = Emesh_ext - e4;

        torch::Tensor cond2 = (Emesh_ext > e1) & (Emesh_ext <= e2);

        torch::Tensor gterm2 = 3.0 * em1.pow(2.0) / (e21 * e31 * e41);

        torch::Tensor vterm21 = -(em2 / e21 + em3 / e31 + em4 / e41) / 3.0;
        torch::Tensor vterm22 = em1 / e21 / 3.0;
        torch::Tensor vterm23 = em1 / e31 / 3.0;
        torch::Tensor vterm24 = em1 / e41 / 3.0;

        torch::Tensor vterm2 = torch::stack({vterm21, vterm22, vterm23, vterm24}, 0);
        vterm2 = (vterm2 * tetgVelTen_sort.unsqueeze(1)).sum(0);

        torch::where_out(pdos, cond2, gterm2, pdos);
        torch::where_out(pvel, cond2, vterm2, pvel);

        torch::Tensor cond3 = (Emesh_ext > e2) & (Emesh_ext <= e3);

        torch::Tensor gterm3 = 3.0 * (em1 + em2 + em2.pow(2.0) * (e1 + e2 - e3 - e4) / (e32 * e42)) / (e31 * e41);

        torch::Tensor vterm31 = (-em4 / e41 - e42 * em1 * em3.pow(2.0) / (e31 * e42 * em1 * em3 + e31.pow(2.0) * em2 * em4)) / 3.0;
        torch::Tensor vterm32 = (-em3 / e32 - e31 * em2 * em4.pow(2.0) / (e31 * e42 * em2 * em4 + e42.pow(2.0) * em1 * em3)) / 3.0;
        torch::Tensor vterm33 = (em2 / e32 + e42 * em3 * em1.pow(2.0) / (e31 * e42 * em1 * em3 + e31.pow(2.0) * em2 * em4)) / 3.0;
        torch::Tensor vterm34 = (em1 / e41 + e31 * em4 * em2.pow(2.0) / (e31 * e42 * em2 * em4 + e42.pow(2.0) * em1 * em3)) / 3.0;

        torch::Tensor vterm3 = torch::stack({vterm31, vterm32, vterm33, vterm34}, 0);
        vterm3 = (vterm3 * tetgVelTen_sort.unsqueeze(1)).sum(0);

        torch::where_out(pdos, cond3, gterm3, pdos);
        torch::where_out(pvel, cond3, vterm3, pvel);

        torch::Tensor cond4 = (Emesh_ext > e3) & (Emesh_ext <= e4);

        torch::Tensor gterm4 = 3.0 * em4.pow(2.0) / (e41 * e42 * e43);

        torch::Tensor vterm41 = -em4 / e41 / 3.0;
        torch::Tensor vterm42 = -em4 / e42 / 3.0;
        torch::Tensor vterm43 = -em4 / e43 / 3.0;
        torch::Tensor vterm44 = (em1 / e41 + em2 / e42 + em3 / e43) / 3.0;

        torch::Tensor vterm4 = torch::stack({vterm41, vterm42, vterm43, vterm44}, 0);
        vterm4 = (vterm4 * tetgVelTen_sort.unsqueeze(1)).sum(0);

        torch::where_out(pdos, cond4, gterm4, pdos);
        torch::where_out(pvel, cond4, vterm4, pvel);

        dos += pdos.sum(1).mul_(tetrahedron_volume_list[i] / total_volume);

        avgVel += (pdos * pvel).sum(1).mul_(tetrahedron_volume_list[i] / total_volume);

        pdos.zero_();
        pvel.zero_();
    }

    pdos = torch::Tensor(); 
    pvel = torch::Tensor(); 

    avgVel /= (dos + 1.0E-10);
    if(gsp != 1.0) dos *= gsp;

    std::cout << "The density of states and the average magnitude of group velocity were calculated." << std::endl;

    std::cout << "Calculating dielectric permittivity..." << std::endl;

    torch::Tensor hwmesh = torch::linspace(0.0, hwmax, static_cast<int64_t>(std::trunc(hwmax / En_step)) + 1L, 
                                           torch::TensorOptions().dtype(torch::kFloat64)).to(device);
                                    
    torch::Tensor hwmesh_ext = hwmesh.unsqueeze(-1);

    const int64_t num_hwpoints = hwmesh.size(0);
    const int64_t num_bpairs = hw_ten_list[0].size(0);

    torch::Tensor Eps2 = torch::zeros({num_hwpoints, 3, 3}, 
                                      torch::TensorOptions().dtype(torch::kFloat64)).to(device);

    torch::Tensor pdos2 = torch::zeros({num_hwpoints, num_bpairs}, 
                                       torch::TensorOptions().dtype(torch::kComplexDouble)).to(device);

    torch::Tensor peps2 = torch::zeros({num_hwpoints, num_bpairs, 3, 3}, 
                                       torch::TensorOptions().dtype(torch::kComplexDouble)).to(device);

    for(size_t i = 0; i < tetrahedron_list.size(); ++i)
    {        
        std::cout << "Tetrahedra: " << i + 1UL << "/" << tetrahedron_list.size() << "\r" << std::flush;

        int64_t v1 = rkvec_to_irrkvec[tetrahedron_list[i][0]];
        int64_t v2 = rkvec_to_irrkvec[tetrahedron_list[i][1]];
        int64_t v3 = rkvec_to_irrkvec[tetrahedron_list[i][2]];
        int64_t v4 = rkvec_to_irrkvec[tetrahedron_list[i][3]];

        torch::Tensor tethwTen = torch::stack({hw_ten_list[v1], hw_ten_list[v2], hw_ten_list[v3], hw_ten_list[v4]}, 0);
        torch::Tensor tetMijTen = torch::stack({Mij_ten_list[v1], Mij_ten_list[v2], Mij_ten_list[v3], Mij_ten_list[v4]}, 0);

        auto [tethwTen_sort, sortIDs] = torch::sort(tethwTen, 0);
        torch::Tensor tetMijTen_sort = torch::take_along_dim(tetMijTen, sortIDs.unsqueeze(-1).unsqueeze(-1), 0);

        torch::Tensor e1 = tethwTen_sort[0].unsqueeze(0) - 2.0 * Constants::en_eps;
        torch::Tensor e2 = tethwTen_sort[1].unsqueeze(0) - Constants::en_eps;
        torch::Tensor e3 = tethwTen_sort[2].unsqueeze(0) + Constants::en_eps;
        torch::Tensor e4 = tethwTen_sort[3].unsqueeze(0) + 2.0 * Constants::en_eps;

        torch::Tensor e21 = e2 - e1;
        torch::Tensor e31 = e3 - e1;
        torch::Tensor e41 = e4 - e1;
        torch::Tensor e32 = e3 - e2;
        torch::Tensor e42 = e4 - e2;
        torch::Tensor e43 = e4 - e3;
        torch::Tensor em1 = hwmesh_ext - e1;
        torch::Tensor em2 = hwmesh_ext - e2;
        torch::Tensor em3 = hwmesh_ext - e3;
        torch::Tensor em4 = hwmesh_ext - e4;

        torch::Tensor cond2 = (hwmesh_ext > e1) & (hwmesh_ext <= e2);

        torch::Tensor gterm2 = 3.0 * em1.pow(2.0) / (e21 * e31 * e41);

        torch::Tensor vterm21 = -(em2 / e21 + em3 / e31 + em4 / e41) / 3.0;
        torch::Tensor vterm22 = em1 / e21 / 3.0;
        torch::Tensor vterm23 = em1 / e31 / 3.0;
        torch::Tensor vterm24 = em1 / e41 / 3.0;

        torch::Tensor vterm2 = torch::stack({vterm21, vterm22, vterm23, vterm24}, 0).to(torch::kComplexDouble);
        vterm2 = torch::einsum("kpn,knij->pnij", {vterm2, tetMijTen_sort});

        torch::where_out(pdos2, cond2, gterm2, pdos2);
        torch::where_out(peps2, cond2.unsqueeze(-1).unsqueeze(-1), vterm2, peps2);

        torch::Tensor cond3 = (hwmesh_ext > e2) & (hwmesh_ext <= e3);

        torch::Tensor gterm3 = 3.0 * (em1 + em2 + em2.pow(2.0) * (e1 + e2 - e3 - e4) / (e32 * e42)) / (e31 * e41);

        torch::Tensor vterm31 = (-em4 / e41 - e42 * em1 * em3.pow(2.0) / (e31 * e42 * em1 * em3 + e31.pow(2.0) * em2 * em4)) / 3.0;
        torch::Tensor vterm32 = (-em3 / e32 - e31 * em2 * em4.pow(2.0) / (e31 * e42 * em2 * em4 + e42.pow(2.0) * em1 * em3)) / 3.0;
        torch::Tensor vterm33 = (em2 / e32 + e42 * em3 * em1.pow(2.0) / (e31 * e42 * em1 * em3 + e31.pow(2.0) * em2 * em4)) / 3.0;
        torch::Tensor vterm34 = (em1 / e41 + e31 * em4 * em2.pow(2.0) / (e31 * e42 * em2 * em4 + e42.pow(2.0) * em1 * em3)) / 3.0;

        torch::Tensor vterm3 = torch::stack({vterm31, vterm32, vterm33, vterm34}, 0).to(torch::kComplexDouble);
        vterm3 = torch::einsum("kpn,knij->pnij", {vterm3, tetMijTen_sort});

        torch::where_out(pdos2, cond3, gterm3, pdos2);
        torch::where_out(peps2, cond3.unsqueeze(-1).unsqueeze(-1), vterm3, peps2);

        torch::Tensor cond4 = (hwmesh_ext > e3) & (hwmesh_ext <= e4);

        torch::Tensor gterm4 = 3.0 * em4.pow(2.0) / (e41 * e42 * e43);

        torch::Tensor vterm41 = -em4 / e41 / 3.0;
        torch::Tensor vterm42 = -em4 / e42 / 3.0;
        torch::Tensor vterm43 = -em4 / e43 / 3.0;
        torch::Tensor vterm44 = (em1 / e41 + em2 / e42 + em3 / e43) / 3.0;

        torch::Tensor vterm4 = torch::stack({vterm41, vterm42, vterm43, vterm44}, 0).to(torch::kComplexDouble);
        vterm4 = torch::einsum("kpn,knij->pnij", {vterm4, tetMijTen_sort});

        torch::where_out(pdos2, cond4, gterm4, pdos2);
        torch::where_out(peps2, cond4.unsqueeze(-1).unsqueeze(-1), vterm4, peps2);

        peps2 *= pdos2.unsqueeze(-1).unsqueeze(-1);

        torch::Tensor result = peps2.sum(1);

        if(use_inv)
        {
            Eps2 += torch::real(result + result.transpose(1, 2).conj()) * tetrahedron_volume_list[i];
        }
        else
        {
            Eps2 += torch::real(result) * tetrahedron_volume_list[i];
        }

        pdos2.zero_();
        peps2.zero_();
    }

    pdos2 = torch::Tensor(); 
    peps2 = torch::Tensor();

    torch::Tensor hwmeshsqr = hwmesh.pow(2);

    Eps2 /= (hwmeshsqr + 1.0E-10).unsqueeze(-1).unsqueeze(-1);

    Eps2 *= gsp * Constants::Ry * Constants::aB * std::pow(Constants::hbar * Constants::c, 4.0) 
          / (M_PI * std::pow(Constants::me, 2.0)) 
          * std::pow(2.0 * M_PI / alat, 3.0) * 1.0E32;

    torch::Tensor Ker = hwmeshsqr.unsqueeze(0) - hwmeshsqr.unsqueeze(1); 

    torch::Tensor denom = Ker.pow(2).add_(std::pow(smearing, 4.0));

    Ker /= denom;
    denom = torch::Tensor(); 
    Ker *= hwmesh.unsqueeze(0);

    torch::Tensor dhwmesh = hwmesh.slice(0, 1, num_hwpoints) - hwmesh.slice(0, 0, num_hwpoints - 1);

    torch::Tensor zero = torch::zeros({1}, hwmesh.options());
    torch::Tensor weights = 0.5 * (torch::cat({zero, dhwmesh}, 0) + torch::cat({dhwmesh, zero}, 0));

    Ker *= weights.unsqueeze(0);

    torch::Tensor Eps1 = torch::einsum("nm,mij->nij", {Ker, Eps2}).mul_(2.0 / M_PI).add_(1.0);
    Ker = torch::Tensor(); 

    std::cout << "Dielectric permittivity was calculated." << std::endl;

    auto finish = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Spent time: " << elapsed.count() << " s." << std::endl;

    c10::cuda::CUDACachingAllocator::emptyCache();

    std::cout << "Saving data..." << std::endl;

    std::ofstream dos_file("DOS.csv");

    if(!dos_file.is_open()) 
    {
        std::cerr << "Error when creating dos.csv file." << std::endl;

        return 1;
    }

    std::ofstream mean_gVel_file("MeanGVel.csv");

    if(!mean_gVel_file.is_open()) 
    {
        std::cerr << "Error when creating avgVel.csv file." << std::endl;

        return 1;
    }

    std::ofstream eps1_file("Eps1.csv");

    if(!eps1_file.is_open()) 
    {
        std::cerr << "Error when creating Eps1.csv file." << std::endl;

        return 1;
    }

    std::ofstream eps2_file("Eps2.csv");

    if(!eps2_file.is_open()) 
    {
        std::cerr << "Error when creating Eps2.csv file." << std::endl;

        return 1;
    }

    torch::Tensor Emesh_cpu = Emesh.cpu();
    torch::Tensor dos_cpu = dos.cpu();
    torch::Tensor avgVel_cpu = avgVel.cpu();
    torch::Tensor hwmesh_cpu = hwmesh.cpu();
    torch::Tensor Eps1_cpu = Eps1.cpu().contiguous();
    torch::Tensor Eps2_cpu = Eps2.cpu().contiguous();

    double* Emesh_ptr = Emesh_cpu.data_ptr<double>();
    double* dos_ptr = dos_cpu.data_ptr<double>();
    double* avgVel_ptr = avgVel_cpu.data_ptr<double>();
    double* hwmesh_ptr = hwmesh_cpu.data_ptr<double>();
    double* Eps1_ptr = Eps1_cpu.data_ptr<double>();
    double* Eps2_ptr = Eps2_cpu.data_ptr<double>();

    double eps1_scalar[num_hwpoints]{0.0};
    double eps2_scalar[num_hwpoints]{0.0};

    dos_file << "energy(eV),dos(eV^-1)\n";

    for(int64_t i = 0; i < num_enpoints; ++i) 
    {
        dos_file << Emesh_ptr[i] << "," << dos_ptr[i] << "\n";
    }

    dos_file.close();

    mean_gVel_file << "energy(eV),vg(cm/s)\n";

    for(int64_t i = 0; i < num_enpoints; ++i) 
    {
        mean_gVel_file << Emesh_ptr[i] << "," << avgVel_ptr[i] << "\n";
    }

    mean_gVel_file.close();

    eps1_file << "energy(eV),eps1_xx,eps1_yy,eps1_zz,eps1_xy,eps1_xz,eps1_yz\n";

    for(int64_t i = 0; i < num_hwpoints; ++i) 
    {
        eps1_file << hwmesh_ptr[i] << "," << Eps1_ptr[i * 9 + 0 * 3 + 0] 
                                   << "," << Eps1_ptr[i * 9 + 1 * 3 + 1] 
                                   << "," << Eps1_ptr[i * 9 + 2 * 3 + 2]
                                   << "," << Eps1_ptr[i * 9 + 0 * 3 + 1]
                                   << "," << Eps1_ptr[i * 9 + 0 * 3 + 2]
                                   << "," << Eps1_ptr[i * 9 + 1 * 3 + 2] << "\n";

        eps1_scalar[i] = (Eps1_ptr[i * 9 + 0 * 3 + 0] + Eps1_ptr[i * 9 + 1 * 3 + 1] + Eps1_ptr[i * 9 + 2 * 3 + 2]) / 3.0;
    }

    eps1_file.close();

    eps2_file << "energy(eV),eps2_xx,eps2_yy,eps2_zz,eps2_xy,eps2_xz,eps2_yz\n";

    for(int64_t i = 0; i < num_hwpoints; ++i) 
    {
        eps2_file << hwmesh_ptr[i] << "," << Eps2_ptr[i * 9 + 0 * 3 + 0] 
                                   << "," << Eps2_ptr[i * 9 + 1 * 3 + 1] 
                                   << "," << Eps2_ptr[i * 9 + 2 * 3 + 2]
                                   << "," << Eps2_ptr[i * 9 + 0 * 3 + 1]
                                   << "," << Eps2_ptr[i * 9 + 0 * 3 + 2]
                                   << "," << Eps2_ptr[i * 9 + 1 * 3 + 2] << "\n";

        eps2_scalar[i] = (Eps2_ptr[i * 9 + 0 * 3 + 0] + Eps2_ptr[i * 9 + 1 * 3 + 1] + Eps2_ptr[i * 9 + 2 * 3 + 2]) / 3.0;
    }

    eps2_file.close();

    std::cout << "Data was saved successfully." << std::endl;

    TApplication app("app", &argc, argv);

    TCanvas* canv = new TCanvas("canv", "Results", 100, 100, 1100, 1100);
    canv->Divide(2, 2);
    canv->cd(1);

    TGraph* dos_graph = new TGraph(num_enpoints, Emesh_ptr, dos_ptr);   
	dos_graph->SetTitle("Density of States");
    dos_graph->SetLineColor(2);
    dos_graph->SetLineWidth(4);
    dos_graph->GetXaxis()->SetTitle("Energy (eV)");
    dos_graph->GetYaxis()->SetTitle("DOS (eV^{-1})");
    dos_graph->GetXaxis()->SetTitleSize(0.04);
    dos_graph->GetXaxis()->SetTitleOffset(1.0);
    dos_graph->GetYaxis()->SetTitleSize(0.04);
    dos_graph->GetYaxis()->SetTitleOffset(1.2);
    dos_graph->Draw("AL");

    canv->cd(2);

    TGraph* gvel_graph = new TGraph(num_enpoints, Emesh_ptr, avgVel_ptr);   
	gvel_graph->SetTitle("Mean Group Velocity");
    gvel_graph->SetLineColor(2);
    gvel_graph->SetLineWidth(4);
    gvel_graph->GetXaxis()->SetTitle("Energy (eV)");
    gvel_graph->GetYaxis()->SetTitle("<v_{g}>_{BZ} (cm/s)");
    gvel_graph->GetXaxis()->SetTitleSize(0.04);
    gvel_graph->GetXaxis()->SetTitleOffset(1.0);
    gvel_graph->GetYaxis()->SetTitleSize(0.04);
    gvel_graph->GetYaxis()->SetTitleOffset(1.2);
    gvel_graph->Draw("AL");

    canv->cd(3);

    TGraph* eps1_graph = new TGraph(num_hwpoints, hwmesh_ptr, eps1_scalar);   
	eps1_graph->SetTitle("Real Part of Dielectric Permittivity");
    eps1_graph->SetLineColor(2);
    eps1_graph->SetLineWidth(4);
    eps1_graph->GetXaxis()->SetTitle("Energy (eV)");
    eps1_graph->GetYaxis()->SetTitle("Re(#varepsilon)");
    eps1_graph->GetXaxis()->SetTitleSize(0.04);
    eps1_graph->GetXaxis()->SetTitleOffset(1.0);
    eps1_graph->GetYaxis()->SetTitleSize(0.04);
    eps1_graph->GetYaxis()->SetTitleOffset(1.2);
    eps1_graph->Draw("AL");

    canv->cd(4);

    TGraph* eps2_graph = new TGraph(num_hwpoints, hwmesh_ptr, eps2_scalar);   
	eps2_graph->SetTitle("Imaginary Part of Dielectric Permittivity");
    eps2_graph->SetLineColor(2);
    eps2_graph->SetLineWidth(4);
    eps2_graph->GetXaxis()->SetTitle("Energy (eV)");
    eps2_graph->GetYaxis()->SetTitle("Im(#varepsilon)");
    eps2_graph->GetXaxis()->SetTitleSize(0.04);
    eps2_graph->GetXaxis()->SetTitleOffset(1.0);
    eps2_graph->GetYaxis()->SetTitleSize(0.04);
    eps2_graph->GetYaxis()->SetTitleOffset(1.2);
    eps2_graph->Draw("AL");

    canv->Modified();
    canv->Update();

    TRootCanvas *rcanv = (TRootCanvas*)canv->GetCanvasImp();
    rcanv->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");

    app.Run();

    return 0;
}