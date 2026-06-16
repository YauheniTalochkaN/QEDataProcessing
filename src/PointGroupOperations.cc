#include "PointGroupOperations.hh"

namespace PointGroupOperations
{
    pointGenerator GetPointGroupOperations(std::string groupName)
    {
        pointGenerator func;
        
        if(groupName == "C1(1)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());

                return result;
            }; 
        }
        else if(groupName == "Ci(-1)")
        { 
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), -vec.Z());

                return result;
            };
        }
        else if(groupName == "C2(2)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), -vec.Z());

                return result;
            };
        }
        else if(groupName == "Cs(m)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), vec.Z());

                return result;
            };
        }
        else if(groupName == "C2h(2/m)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), vec.Z());

                return result;
            };
        }
        else if(groupName == "D2(222)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), -vec.Z());

                return result;
            };
        }
        else if(groupName == "C2v(mm2)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), vec.Z());

                return result;
            };
        }
        else if(groupName == "D2h(mmm)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), vec.Z());

                return result;
            };
        }
        else if(groupName == "C4(4)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X(), vec.Z());
                result.emplace_back(vec.Y(), -vec.X(), vec.Z());

                return result;
            };
        }
        else if(groupName == "S4(-4)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(vec.Y(), -vec.X(), -vec.Z());
                result.emplace_back(-vec.Y(), vec.X(), -vec.Z());

                return result;
            };
        }
        else if(groupName == "C4h(4/m)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X(), vec.Z());
                result.emplace_back(vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(vec.Y(), -vec.X(), -vec.Z());
                result.emplace_back(-vec.Y(), vec.X(), -vec.Z());

                return result;
            };
        }
        else if(groupName == "D4(422)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X(), vec.Z());
                result.emplace_back(vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(vec.Y(), vec.X(), -vec.Z());
                result.emplace_back(-vec.Y(), -vec.X(), -vec.Z());

                return result;
            };
        }
        else if(groupName == "C4v(4mm)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X(), vec.Z());
                result.emplace_back(vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(vec.Y(), vec.X(), vec.Z());

                return result;
            };
        }
        else if(groupName == "D2d(-42m)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(vec.Y(), -vec.X(), -vec.Z());
                result.emplace_back(-vec.Y(), vec.X(), -vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(-vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(vec.Y(), vec.X(), vec.Z());

                return result;
            };
        }
        else if(groupName == "D4h(4/mmm)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X(), vec.Z());
                result.emplace_back(vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(vec.Y(), vec.X(), -vec.Z());
                result.emplace_back(-vec.Y(), -vec.X(), -vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(vec.Y(), -vec.X(), -vec.Z());
                result.emplace_back(-vec.Y(), vec.X(), -vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(vec.Y(), vec.X(), vec.Z());

                return result;
            };
        }
        else if(groupName == "C3(3)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), vec.Z());

                return result;
            };
        }
        else if(groupName == "C3i(-3)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(vec.Y(), -vec.X() + vec.Y(), -vec.Z());
                result.emplace_back(vec.X() - vec.Y(), vec.X(), -vec.Z());

                return result;
            };
        }
        else if(groupName == "D3(32)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.Y(), -vec.X(), -vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), vec.X() - vec.Y(), -vec.Z());

                return result;
            };
        }
        else if(groupName == "C3v(3m)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), vec.Y(), vec.Z());
                result.emplace_back(vec.X(), vec.X() - vec.Y(), vec.Z());

                return result;
            };
        }
        else if(groupName == "D3d(-3m)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.Y(), -vec.X(), -vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), vec.X() - vec.Y(), -vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(vec.Y(), -vec.X() + vec.Y(), -vec.Z());
                result.emplace_back(vec.X() - vec.Y(), vec.X(), -vec.Z());
                result.emplace_back(vec.Y(), vec.X(), vec.Z());
                result.emplace_back(vec.X() - vec.Y(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.X() + vec.Y(), vec.Z());

                return result;
            };
        }
        else if(groupName == "C6(6)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(vec.Y(), -vec.X() + vec.Y(), vec.Z());
                result.emplace_back(vec.X() - vec.Y(), vec.X(), vec.Z());

                return result;
            };
        }
        else if(groupName == "C3h(-6)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), -vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), -vec.Z());

                return result;
            };
        }
        else if(groupName == "C6h(6/m)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(vec.Y(), -vec.X() + vec.Y(), vec.Z());
                result.emplace_back(vec.X() - vec.Y(), vec.X(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(vec.Y(), -vec.X() + vec.Y(), -vec.Z());
                result.emplace_back(vec.X() - vec.Y(), vec.X(), -vec.Z());
                result.emplace_back(vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), -vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), -vec.Z());

                return result;
            };
        }
        else if(groupName == "D6(622)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(vec.Y(), -vec.X() + vec.Y(), vec.Z());
                result.emplace_back(vec.X() - vec.Y(), vec.X(), vec.Z());
                result.emplace_back(vec.Y(), vec.X(), -vec.Z());
                result.emplace_back(vec.X() - vec.Y(), -vec.Y(), -vec.Z());
                result.emplace_back(-vec.X(), -vec.X() + vec.Y(), -vec.Z());
                result.emplace_back(-vec.Y(), -vec.X(), -vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), vec.X() - vec.Y(), -vec.Z());

                return result;
            };
        }
        else if(groupName == "C6v(6mm)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(vec.Y(), -vec.X() + vec.Y(), vec.Z());
                result.emplace_back(vec.X() - vec.Y(), vec.X(), vec.Z());
                result.emplace_back(-vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), vec.Y(), vec.Z());
                result.emplace_back(vec.X(), vec.X() - vec.Y(), vec.Z());
                result.emplace_back(vec.Y(), vec.X(), vec.Z());
                result.emplace_back(vec.X() - vec.Y(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.X() + vec.Y(), vec.Z());

                return result;
            };
        }
        else if(groupName == "D3h(-6m2)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), -vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), -vec.Z());
                result.emplace_back(vec.Y(), vec.X(), -vec.Z());
                result.emplace_back(vec.X() - vec.Y(), -vec.Y(), -vec.Z());
                result.emplace_back(-vec.X(), -vec.X() + vec.Y(), -vec.Z());
                result.emplace_back(vec.Y(), vec.X(), vec.Z());
                result.emplace_back(vec.X() - vec.Y(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.X() + vec.Y(), vec.Z());

                return result;
            };
        }
        else if(groupName == "D6h(6/mmm)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(vec.Y(), -vec.X() + vec.Y(), vec.Z());
                result.emplace_back(vec.X() - vec.Y(), vec.X(), vec.Z());
                result.emplace_back(vec.Y(), vec.X(), -vec.Z());
                result.emplace_back(vec.X() - vec.Y(), -vec.Y(), -vec.Z());
                result.emplace_back(-vec.X(), -vec.X() + vec.Y(), -vec.Z());
                result.emplace_back(-vec.Y(), -vec.X(), -vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), vec.X() - vec.Y(), -vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(vec.Y(), -vec.X() + vec.Y(), -vec.Z());
                result.emplace_back(vec.X() - vec.Y(), vec.X(), -vec.Z());
                result.emplace_back(vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(-vec.Y(), vec.X() - vec.Y(), -vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), -vec.X(), -vec.Z());
                result.emplace_back(-vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.X() + vec.Y(), vec.Y(), vec.Z());
                result.emplace_back(vec.X(), vec.X() - vec.Y(), vec.Z());
                result.emplace_back(vec.Y(), vec.X(), vec.Z());
                result.emplace_back(vec.X() - vec.Y(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.X() + vec.Y(), vec.Z());

                return result;
            };
        }
        else if(groupName == "T(23)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(vec.Z(), vec.X(), vec.Y());
                result.emplace_back(vec.Z(), -vec.X(), -vec.Y());
                result.emplace_back(-vec.Z(), -vec.X(), vec.Y());
                result.emplace_back(-vec.Z(), vec.X(), -vec.Y());
                result.emplace_back(vec.Y(), vec.Z(), vec.X());
                result.emplace_back(-vec.Y(), vec.Z(), -vec.X());
                result.emplace_back(vec.Y(), -vec.Z(), -vec.X());
                result.emplace_back(-vec.Y(), -vec.Z(), vec.X());

                return result;
            };
        }
        else if(groupName == "Th(m-3)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(vec.Z(), vec.X(), vec.Y());
                result.emplace_back(vec.Z(), -vec.X(), -vec.Y());
                result.emplace_back(-vec.Z(), -vec.X(), vec.Y());
                result.emplace_back(-vec.Z(), vec.X(), -vec.Y());
                result.emplace_back(vec.Y(), vec.Z(), vec.X());
                result.emplace_back(-vec.Y(), vec.Z(), -vec.X());
                result.emplace_back(vec.Y(), -vec.Z(), -vec.X());
                result.emplace_back(-vec.Y(), -vec.Z(), vec.X());
                result.emplace_back(-vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.Z(), -vec.X(), -vec.Y());
                result.emplace_back(-vec.Z(), vec.X(), vec.Y());
                result.emplace_back(vec.Z(), vec.X(), -vec.Y());
                result.emplace_back(vec.Z(), -vec.X(), vec.Y());
                result.emplace_back(-vec.Y(), -vec.Z(), -vec.X());
                result.emplace_back(vec.Y(), -vec.Z(), vec.X());
                result.emplace_back(-vec.Y(), vec.Z(), vec.X());
                result.emplace_back(vec.Y(), vec.Z(), -vec.X());

                return result;
            };
        }
        else if(groupName == "O(432)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(vec.Z(), vec.X(), vec.Y());
                result.emplace_back(vec.Z(), -vec.X(), -vec.Y());
                result.emplace_back(-vec.Z(), vec.X(), -vec.Y());
                result.emplace_back(-vec.Z(), -vec.X(), vec.Y());
                result.emplace_back(vec.Y(), vec.Z(), vec.X());
                result.emplace_back(-vec.Y(), vec.Z(), -vec.X());
                result.emplace_back(-vec.Y(), -vec.Z(), vec.X());
                result.emplace_back(vec.Y(), -vec.Z(), -vec.X());
                result.emplace_back(vec.Y(), vec.X(), -vec.Z());
                result.emplace_back(-vec.Y(), -vec.X(), -vec.Z());
                result.emplace_back(-vec.Y(), vec.X(), vec.Z());
                result.emplace_back(vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(vec.X(), vec.Z(), -vec.Y());
                result.emplace_back(-vec.X(), vec.Z(), vec.Y());
                result.emplace_back(vec.X(), -vec.Z(), vec.Y());
                result.emplace_back(-vec.X(), -vec.Z(), -vec.Y());
                result.emplace_back(vec.Z(), vec.Y(), -vec.X());
                result.emplace_back(vec.Z(), -vec.Y(), vec.X());
                result.emplace_back(-vec.Z(), -vec.Y(), -vec.X());
                result.emplace_back(-vec.Z(), vec.Y(), vec.X());

                return result;
            };
        }
        else if(groupName == "Td(-43m)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(vec.Z(), vec.X(), vec.Y());
                result.emplace_back(vec.Z(), -vec.X(), -vec.Y());
                result.emplace_back(-vec.Z(), vec.X(), -vec.Y());
                result.emplace_back(-vec.Z(), -vec.X(), vec.Y());
                result.emplace_back(vec.Y(), vec.Z(), vec.X());
                result.emplace_back(-vec.Y(), vec.Z(), -vec.X());
                result.emplace_back(-vec.Y(), -vec.Z(), vec.X());
                result.emplace_back(vec.Y(), -vec.Z(), -vec.X());
                result.emplace_back(vec.Y(), vec.X(), vec.Z());
                result.emplace_back(-vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(-vec.Y(), vec.X(), -vec.Z());
                result.emplace_back(vec.Y(), -vec.X(), -vec.Z());
                result.emplace_back(vec.X(), vec.Z(), vec.Y());
                result.emplace_back(-vec.X(), vec.Z(), -vec.Y());
                result.emplace_back(vec.X(), -vec.Z(), -vec.Y());
                result.emplace_back(-vec.X(), -vec.Z(), vec.Y());
                result.emplace_back(vec.Z(), vec.Y(), vec.X());
                result.emplace_back(vec.Z(), -vec.Y(), -vec.X());
                result.emplace_back(-vec.Z(), -vec.Y(), vec.X());
                result.emplace_back(-vec.Z(), vec.Y(), -vec.X());

                return result;
            };
        }
        else if(groupName == "Oh(m-3m)")
        {
            func = [](const TVector3& vec) -> std::vector<TVector3>
            {
                std::vector<TVector3> result;

                result.emplace_back(vec.X(), vec.Y(), vec.Z());
                result.emplace_back(-vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(vec.Z(), vec.X(), vec.Y());
                result.emplace_back(vec.Z(), -vec.X(), -vec.Y());
                result.emplace_back(-vec.Z(), vec.X(), -vec.Y());
                result.emplace_back(-vec.Z(), -vec.X(), vec.Y());
                result.emplace_back(vec.Y(), vec.Z(), vec.X());
                result.emplace_back(-vec.Y(), vec.Z(), -vec.X());
                result.emplace_back(-vec.Y(), -vec.Z(), vec.X());
                result.emplace_back(vec.Y(), -vec.Z(), -vec.X());
                result.emplace_back(vec.Y(), vec.X(), -vec.Z());
                result.emplace_back(-vec.Y(), -vec.X(), -vec.Z());
                result.emplace_back(-vec.Y(), vec.X(), vec.Z());
                result.emplace_back(vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(vec.X(), vec.Z(), -vec.Y());
                result.emplace_back(-vec.X(), vec.Z(), vec.Y());
                result.emplace_back(vec.X(), -vec.Z(), vec.Y());
                result.emplace_back(-vec.X(), -vec.Z(), -vec.Y());
                result.emplace_back(vec.Z(), vec.Y(), -vec.X());
                result.emplace_back(vec.Z(), -vec.Y(), vec.X());
                result.emplace_back(-vec.Z(), -vec.Y(), -vec.X());
                result.emplace_back(-vec.Z(), vec.Y(), vec.X());
                result.emplace_back(-vec.X(), -vec.Y(), -vec.Z());
                result.emplace_back(vec.X(), vec.Y(), -vec.Z());
                result.emplace_back(-vec.X(), vec.Y(), vec.Z());
                result.emplace_back(vec.X(), -vec.Y(), vec.Z());
                result.emplace_back(-vec.Z(), -vec.X(), -vec.Y());
                result.emplace_back(-vec.Z(), vec.X(), vec.Y());
                result.emplace_back(vec.Z(), -vec.X(), vec.Y());
                result.emplace_back(vec.Z(), vec.X(), -vec.Y());
                result.emplace_back(-vec.Y(), -vec.Z(), -vec.X());
                result.emplace_back(vec.Y(), -vec.Z(), vec.X());
                result.emplace_back(vec.Y(), vec.Z(), -vec.X());
                result.emplace_back(-vec.Y(), vec.Z(), vec.X());
                result.emplace_back(-vec.Y(), -vec.X(), vec.Z());
                result.emplace_back(vec.Y(), vec.X(), vec.Z());
                result.emplace_back(vec.Y(), -vec.X(), -vec.Z());
                result.emplace_back(-vec.Y(), vec.X(), -vec.Z());
                result.emplace_back(-vec.X(), -vec.Z(), vec.Y());
                result.emplace_back(vec.X(), -vec.Z(), -vec.Y());
                result.emplace_back(-vec.X(), vec.Z(), -vec.Y());
                result.emplace_back(vec.X(), vec.Z(), vec.Y());
                result.emplace_back(-vec.Z(), -vec.Y(), vec.X());
                result.emplace_back(-vec.Z(), vec.Y(), -vec.X());
                result.emplace_back(vec.Z(), vec.Y(), vec.X());
                result.emplace_back(vec.Z(), -vec.Y(), -vec.X());

                return result;
            };
        }

        return func;
    }
}