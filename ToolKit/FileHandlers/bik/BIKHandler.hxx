
#include "FileHandler.hxx"

class BIKHandler : public FileHandler
{
public:
    void LoadFile()
    {
        std::wcout << "Loading BIK file: " << std::endl;
        // Add CSO file loading logic here
    }

    void RenderTree()
    {
    }

    void RenderPropetries()
    {
    }

    void RenderHex()
    {
    }

    void Render()
    {
        printf("Render BIK Layout\n");
    }
};