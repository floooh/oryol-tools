//------------------------------------------------------------------------------
//  N3Loader.cc
//------------------------------------------------------------------------------
#include "N3Loader.h"
#include "ExportUtil/Log.h"
#include "pystring.h"
#include <stdio.h>
#include <set>

using namespace OryolTools;

//------------------------------------------------------------------------------
template<typename TYPE> TYPE read(FILE* fp) {
    TYPE val;
    int bytesRead = fread(&val, 1, sizeof(val), fp);
    Log::FailIf(bytesRead != sizeof(val), "File read error.");
    return val;
};

//------------------------------------------------------------------------------
template<> std::string read<std::string>(FILE* fp) {
    uint16_t len = read<uint16_t>(fp);
    Log::FailIf(len > 4096, "Long string in file.");
    std::string str;
    str.reserve(len);
    for (uint16_t i = 0; i < len; i++) {
        str.push_back(read<char>(fp));
    }
    return str;
}

//------------------------------------------------------------------------------
void
N3Loader::Load(const std::string& n3AssetName, const std::string& n3AssetDir, IRep& irep) {
    Log::FailIf(n3AssetDir.empty(), "N3 asset root directory required!\n");
    Log::FailIf(n3AssetName.empty(), "N3 asset name required!\n");

    std::string path = n3AssetDir + "/models/" + n3AssetName;

    this->LoadN3(path);
    this->LoadMeshes(n3AssetDir);
}

//------------------------------------------------------------------------------
void
N3Loader::LoadN3(const std::string& path) {
    FILE* fp = fopen(path.c_str(), "rb");
    Log::FailIf(!fp, "Failed to open N3 file '%s'\n", path.c_str());

    // check magic number and version
    const uint32_t magic = read<uint32_t>(fp);
    const uint32_t version = read<uint32_t>(fp);
    Log::FailIf(magic != 'NEB3', "Not a .n3 file\n");
    Log::FailIf(version != 2, "Unknown .n3 file version\n");

    // start reading tags (trying to read past end of file would throw a fatal error)
    bool done = false;
    while (!done) {
        const uint32_t tag = read<uint32_t>(fp);
        switch (tag) {
            // outermost begin-model tag
            case '>MDL':
                read<uint32_t>(fp); // skip model class tag
                this->ModelName = read<std::string>(fp);
                break;
            // outermost end-model tag (last thing in file)
            case '<MDL':
                done = true;
                break;
            // start of a new model node
            case '>MND':
                {
                    N3Node node;
                    node.ClassTag = read<uint32_t>(fp);
                    node.Name = read<std::string>(fp);
                    node.Parent = this->NodeIndexStack.empty() ? -1 : this->NodeIndexStack.back();
                    this->NodeIndexStack.push_back(this->Nodes.size());
                    if (node.Parent != -1) {
                        this->Nodes[node.Parent].Children.push_back(this->Nodes.size());
                    }
                    this->Nodes.push_back(node);
                }
                break;
            // end of current model node
            case '<MND':
                this->NodeIndexStack.pop_back();
                break;
            // generic data tag
            default:
                switch (this->Nodes.back().ClassTag) {
                    case 'TRFN':    this->ParseTransformNodeTag(fp, tag); break;
                    case 'SPND':    this->ParseShapeNodeTag(fp, tag); break;
                    case 'MANI':    this->ParseAnimatorNodeTag(fp, tag); break;
                    case 'PSND':    this->ParseParticleSystemNodeTag(fp, tag); break;
                    case 'CHRN':    this->ParseCharacterNodeTag(fp, tag); break;
                    case 'CHSN':    this->ParseCharacterSkinNodeTag(fp, tag); break;
                    default:        Log::Fatal("Unknown class tag in .n3 file\n");
                }
                break;
        }
    }
    fclose(fp);
}

//------------------------------------------------------------------------------
void
N3Loader::ParseModelNodeTag(FILE* fp, uint32_t tag) {
    N3Node& node = this->Nodes.back();
    switch (tag) {
        case 'LBOX':
            node.Center = read<glm::vec4>(fp);
            node.Extents = read<glm::vec4>(fp);
            break;
        case 'MNTP':
            node.NodeType = read<std::string>(fp);
            break;
        case 'SSTA':
            // skip string key/value pairs
            read<std::string>(fp);
            read<std::string>(fp);
            break;
        case 'CASH':
            // skip cast-shadows
            read<bool>(fp);
            break;
        case 'HRCH':
            // skip hierarchy-node-flag
            read<bool>(fp);
            break;
        default:
            // unknown tag
            Log::Fatal("Unknown tag in .n3 file: 0x%4X\n", tag);
            break;
    }
}

//------------------------------------------------------------------------------
void
N3Loader::ParseTransformNodeTag(FILE* fp, uint32_t tag) {
    N3Node& node = this->Nodes.back();
    switch (tag) {
        case 'POSI':
            node.Position = read<glm::vec4>(fp);
            break;
        case 'ROTN':
            node.Rotation = read<glm::vec4>(fp);
            break;
        case 'SCAL':
            node.Scaling = read<glm::vec4>(fp);
            break;
        case 'RPIV':
            node.RotatePivot = read<glm::vec4>(fp);
            break;
        case 'SPIV':
            node.ScalePivot = read<glm::vec4>(fp);
            break;
        case 'SVSP':
        case 'SLKV':
        case 'SMID':
        case 'SMAD':
        case 'SSPR':
        case 'SBLB':
            // skip transform node flags
            read<bool>(fp);
            break;
        default:
            this->ParseModelNodeTag(fp, tag);
            break;
    } 
}

//------------------------------------------------------------------------------
void
N3Loader::ParseStateNodeTag(FILE* fp, uint32_t tag) {
    N3Node& node = this->Nodes.back();
    std::string paramName, strValue;
    int intValue = 0;
    float floatValue = 0.0f;
    glm::vec4 vecValue;
    switch (tag) {
        case 'SHDR':
            node.Shader = read<std::string>(fp);
            node.Shader = pystring::replace(node.Shader, "shd:", "", 1);
            break;
        case 'STXT':
            paramName = read<std::string>(fp);
            strValue = read<std::string>(fp);
            strValue = pystring::replace(strValue, "tex:", "", 1);
            node.Textures[paramName] = strValue;
            break;
        case 'SINT':
            paramName = read<std::string>(fp); 
            intValue = read<int>(fp);
            node.IntParams[paramName] = intValue;
            break;
        case 'SFLT':
            paramName = read<std::string>(fp);
            floatValue = read<float>(fp);
            node.FloatParams[paramName] = floatValue;
            break;
        case 'SVEC':
            paramName = read<std::string>(fp);
            vecValue = read<glm::vec4>(fp);
            node.VecParams[paramName] = vecValue;
            break;
        case 'STUS':
        case 'SSPI':
            // skip multilayer params
            read<int>(fp);
            read<glm::vec4>(fp);
            break;
        default:
            this->ParseTransformNodeTag(fp, tag);
            break;
    }
}

//------------------------------------------------------------------------------
void
N3Loader::ParseShapeNodeTag(FILE* fp, uint32_t tag) {
    N3Node& node = this->Nodes.back();
    switch (tag) {
        case 'MESH':
            node.Mesh = read<std::string>(fp);
            node.Mesh = pystring::replace(node.Mesh, "msh:", "", 1);
            break;
        case 'PGRI':
            node.PrimGroup = read<int>(fp);
            break;
        default:
            this->ParseStateNodeTag(fp, tag);
            break;
    }
}

//------------------------------------------------------------------------------
void
N3Loader::ParseAnimatorNodeTag(FILE* fp, uint32_t tag) {
    // animator nodes not supported, just skip everything
    static int nodeType = 0;
    int animKeySize = 0;
    std::string valueType;
    switch (tag) {
        case 'BASE':
            nodeType = read<int>(fp);
            break;
        case 'SLPT':
        case 'ANNO':
        case 'SPNM':
        case 'SVCN':
        case 'SANI':
            read<std::string>(fp);
            break;
        case 'SAGR':
            read<int>(fp);
            break;
        case 'ADPK':
        case 'ADEK':
        case 'ADSK':
            animKeySize = read<int>(fp);
            for (int i = 0; i < animKeySize; i++) {
                if (nodeType == 5) {
                    read<int>(fp);
                }
                for (int j = 0; j < 5; j++) {
                    read<float>(fp);
                }
            }
            break;
        case 'ADDK':
            valueType = read<std::string>(fp);
            animKeySize = read<int>(fp);
            for (int i = 0; i < animKeySize; i++) {
                read<float>(fp);
                if (valueType == "Float") {
                    read<float>(fp);
                }
                else if (valueType == "Float4") {
                    read<glm::vec4>(fp);
                }
                else if (valueType == "Int") {
                    read<int>(fp);
                }
                else {
                    Log::Fatal("Unknown animator node key type\n");
                }
            }
            break;
    }
}

//------------------------------------------------------------------------------
static void skipEnvelopeCurve(FILE* fp) {
    for (int i = 0; i < 8; i++) {
        read<float>(fp);
    }
    read<int>(fp);
}

//------------------------------------------------------------------------------
void
N3Loader::ParseParticleSystemNodeTag(FILE* fp, uint32_t tag) {
    // skip all particle system tags
    switch (tag) {
        case 'EFRQ':
        case 'PLFT':
        case 'PSMN':
        case 'PSMX':
        case 'PSVL':
        case 'PRVL':
        case 'PSZE':
        case 'PMSS':
        case 'PTMN':
        case 'PVLF':
        case 'PAIR':
        case 'PRED':
        case 'PGRN':
        case 'PBLU':
        case 'PALP':
            skipEnvelopeCurve(fp);
            break;
        case 'PEDU':
        case 'PACD':
        case 'PRMN':
        case 'PRMX':
        case 'PGRV':
        case 'PSTC':
        case 'PTTX':
        case 'PVRM':
        case 'PRRM':
        case 'PSRM':
        case 'PPCT':
        case 'PDEL':
            read<float>(fp);
            break;
        case 'PLPE':
        case 'PROF':
        case 'PBBO':
        case 'PSTS':
        case 'PRRD':
        case 'PSDL':
        case 'PVAF':
        case 'PGRI':
            read<int>(fp);
            break;
        default:
            this->ParseStateNodeTag(fp, tag);
            break;
    }
}

//------------------------------------------------------------------------------
void
N3Loader::ParseCharacterNodeTag(FILE* fp, uint32_t tag) {
    N3Node& node = this->Nodes.back();
    switch (tag) {
        case 'ANIM':
            node.Animation = read<std::string>(fp);
            node.Animation = pystring::replace(node.Animation, "ani:", "", 1);
            break;
        case 'NJNT':
            // skip NumJoints
            read<int>(fp);
            break;
        case 'JONT':
            {
                node.Joints.push_back(N3Node::Joint());
                auto& joint = node.Joints.back();
                read<int>(fp);  // skip joint index
                joint.Parent = read<int>(fp);
                joint.PoseTranslation = read<glm::vec4>(fp);
                joint.PoseRotation = read<glm::vec4>(fp);
                joint.PoseScale = read<glm::vec4>(fp);
                joint.Name = read<std::string>(fp);
            };
            break;
        case 'VART':
            // skip variation name
            read<std::string>(fp);
            break;
        case 'NSKL':
            // skip NumSkinLists
            read<int>(fp);
            break;
        case 'SKNL':
            // skip SkinLists
            {
                read<std::string>(fp);
                const int num = read<int>(fp);
                for (int i = 0; i < num; i++) {
                    read<std::string>(fp);
                }
                const bool hasVariation = read<bool>(fp);
                if (hasVariation) {
                    read<std::string>(fp);
                }
            }
            break;
        default:
            this->ParseTransformNodeTag(fp, tag);
            break;
    }
}

//------------------------------------------------------------------------------
void
N3Loader::ParseCharacterSkinNodeTag(FILE* fp, uint32_t tag) {
    N3Node& node = this->Nodes.back();
    switch (tag) {
        case 'NSKF':
            // skin NumSkinFragments
            read<int>(fp);
            break;
        case 'SFRG':
            {
                node.SkinFragments.push_back(N3Node::SkinFragment());
                auto& frag = node.SkinFragments.back();
                frag.PrimGroup = read<int>(fp);
                const int numJoints = read<int>(fp);
                for (int i = 0; i < numJoints; i++) {
                    frag.JointPalette.push_back(read<int>(fp));
                }
            }
            break;
        default:
            this->ParseShapeNodeTag(fp, tag);
            break;
    }
}

//------------------------------------------------------------------------------
#pragma pack(push, 1)
struct Nvx2Header {
    uint32_t Magic = 0;
    uint32_t NumGroups = 0;
    uint32_t NumVertices = 0;
    uint32_t VertexWidth = 0;
    uint32_t NumTriangles = 0;
    uint32_t NumEdges = 0;
    uint32_t VertexComponentMask = 0;
};
struct Nvx2Group {
    uint32_t FirstVertex = 0;
    uint32_t NumVertices = 0;
    uint32_t FirstTriangle = 0;
    uint32_t NumTriangles = 0;
    uint32_t FirstEgde = 0;
    uint32_t NumEdges = 0;
};
#pragma pack(pop)

void
N3Loader::LoadMeshes(const std::string& n3AssetDir) {

    // for each node with mesh data...
    std::set<std::string> loadedMeshes;
    for (const auto& node : this->Nodes) {
        if (node.Mesh.empty()) {
            continue;   // no mesh in this node
        }
        if (loadedMeshes.find(node.Mesh) != loadedMeshes.end()) {
            continue;   // mesh already loaded, skip this node
        }
        loadedMeshes.insert(node.Mesh);
        std::string meshPath = n3AssetDir + "/meshes/" + node.Mesh;
        FILE* fp = fopen(meshPath.c_str(), "rb");
        Log::FailIf(!fp, "Failed to open mesh file '%s'\n", meshPath.c_str());

        // load header data
        Nvx2Header hdr = read<Nvx2Header>(fp);
        Log::FailIf(hdr.Magic != 'NVX2', "Magic number mismatch in '%s'\n", meshPath.c_str());

        fclose(fp);
    }
}
