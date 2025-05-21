#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <windows.h>  // For GetModuleFileNameA
#include <cstdio>     // For std::remove

// Replace backslashes with forward slashes for VMAT consistency
std::string fixPathForMaterials(const std::string& path) {
    std::string fixed = path;
    std::replace(fixed.begin(), fixed.end(), '\\', '/');
    return fixed;
}

std::string generateVmatContent(const std::string& textureName) {
    return R"("Layer0"
{
	"shader"		"shaders/complex.shader"
	"g_flAmbientOcclusionDirectDiffuse"		"0.000000"
	"g_flAmbientOcclusionDirectSpecular"		"0.000000"
	"TextureAmbientOcclusion"		"materials/default/default_ao.tga"
	"g_flModelTintAmount"		"1.000000"
	"g_vColorTint"		"[1.000000 1.000000 1.000000 0.000000]"
	"TextureColor"		"materials/)" + textureName + R"("
	"g_flFadeExponent"		"1.000000"
	"g_bFogEnabled"		"1"
	"g_flMetalness"		"0.000000"
	"TextureNormal"		"materials/default/default_normal.tga"
	"g_flRoughnessScaleFactor"		"1.000000"
	"TextureRoughness"		"materials/default/default_rough.tga"
	"g_nScaleTexCoordUByModelScaleAxis"		"0"
	"g_nScaleTexCoordVByModelScaleAxis"		"0"
	"g_vTexCoordOffset"		"[0.000 0.000]"
	"g_vTexCoordScale"		"[1.000 1.000]"
	"g_vTexCoordScrollSpeed"		"[0.000 0.000]"
}
)";
}

int main() {
    // Get all PNGs in current folder and subfolders
    system("dir /s /b *.png > filelist.txt");

    std::ifstream fileList("filelist.txt");
    if (!fileList.is_open()) {
        std::cerr << "Failed to open filelist.txt" << std::endl;
        return 1;
    }

    std::string fullPath;

    // Get the folder path where the executable resides
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string basePath = std::string(exePath);
    basePath = basePath.substr(0, basePath.find_last_of("\\/") + 1);

    while (std::getline(fileList, fullPath)) {
        // Get relative path by stripping basePath from fullPath
        if (fullPath.compare(0, basePath.size(), basePath) != 0) {
            // If somehow fullPath doesn't start with basePath, just use fullPath directly
            std::cerr << "Warning: file path does not start with exe path base: " << fullPath << std::endl;
            continue;
        }

        std::string relativePath = fullPath.substr(basePath.size());

        // Fix slashes and prepare VMAT path
        std::string textureName = fixPathForMaterials(relativePath);

        // Construct .vmat filename in the same directory as the PNG
        std::string vmatPath = fullPath.substr(0, fullPath.find_last_of('.')) + ".vmat";

        // Check if VMAT file already exists - skip if yes
        std::ifstream checkFile(vmatPath);
        if (checkFile.good()) {
            std::cout << "Skipping existing: " << vmatPath << std::endl;
            checkFile.close();
            continue;
        }
        checkFile.close();

        // Create VMAT file
        std::ofstream outFile(vmatPath);
        if (outFile.is_open()) {
            outFile << generateVmatContent(textureName);
            outFile.close();
            std::cout << "Created: " << vmatPath << std::endl;
        } else {
            std::cerr << "Failed to write file: " << vmatPath << std::endl;
        }
    }

    fileList.close();
    std::remove("filelist.txt");

    return 0;
}
