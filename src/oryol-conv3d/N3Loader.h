#pragma once
//------------------------------------------------------------------------------
/**
    @class N3Loader
    @brief load .n3 file and dependent resources
*/
#include <string>
#include <map>
#include <vector>
#include <stdio.h>
#include <glm/vec4.hpp>
#include "IRep.h"
#include "NVX2Loader.h"
#include "NAX3Loader.h"

struct N3Loader {
    /// load a file into intermediate representation
    void Load(const std::string& n3AssetName, const std::string& n3AssetDir, IRep& irep);

    /// load N3 model into internal data structures
    void loadN3(const std::string& path);
    /// load NVX2 mesh data
    void loadMeshes(const std::string& n3AssetDir);
    /// load NAX3 animation data
    void loadAnims(const std::string& n3AssetDir);
    /// write loading result into intermediate representation
    void toIRep(IRep& irep);

    /// parse a top-level ModelNode tag
    void parseModelNodeTag(FILE* fp, uint32_t tag);
    /// parse a TransformNode tag
    void parseTransformNodeTag(FILE* fp, uint32_t tag);
    /// parse a StateNode tag
    void parseStateNodeTag(FILE* fp, uint32_t tag);
    /// parse a ShapeNode tag
    void parseShapeNodeTag(FILE* fp, uint32_t tag);
    /// parse an AnimatorNode tag
    void parseAnimatorNodeTag(FILE* fp, uint32_t tag);
    /// parse a ParticleSystemNode tag
    void parseParticleSystemNodeTag(FILE* fp, uint32_t tag);
    /// parse a CharacterNode tag
    void parseCharacterNodeTag(FILE* fp, uint32_t tag);
    /// parse a CharacterSkinNode tag
    void parseCharacterSkinNodeTag(FILE* fp, uint32_t tag);

    struct N3Node {
        uint32_t ClassTag = 0;
        std::string Name;
        int32_t Parent = -1;
        std::string NodeType;
        std::vector<int> Children;
        glm::vec4 Center = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec4 Extents = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        glm::vec4 Position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec4 Rotation = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec4 Scaling = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        glm::vec4 RotatePivot = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec4 ScalePivot = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        std::string Shader;
        std::map<std::string, std::string> Textures;
        std::map<std::string, int> IntParams;
        std::map<std::string, float> FloatParams;
        std::map<std::string, glm::vec4> VecParams;
        std::string Mesh;
        int PrimGroup = 0;
        std::string Animation;
        struct Joint {
            int Parent = -1;
            glm::vec4 PoseTranslation = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            glm::vec4 PoseRotation = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);;
            glm::vec4 PoseScale = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
            std::string Name;
        };
        std::vector<Joint> Joints;
        struct SkinFragment {
            int PrimGroup = 0;
            std::vector<int> JointPalette;
        };
        std::vector<SkinFragment> SkinFragments;
    };

    std::string ModelName;
    std::vector<N3Node> Nodes;
    std::vector<int> NodeIndexStack;
    NVX2Loader nvx2Loader;
    NAX3Loader nax3Loader;
};

