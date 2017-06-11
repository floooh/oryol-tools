#pragma once
//------------------------------------------------------------------------------
/**
    @class NAX3Loader
    @brief loader for .nax3 animation files
*/
#include <string>
#include <vector>

struct NAX3Loader {

    /// load an .nax3 main file and all its clips
    void Load(const std::string& nax3AssetName, const std::string& n3AssetDir);
    /// clear the loader
    void Clear();
    /// check that the loaded data is valid
    void Validate();

    /// an animation keys
    struct Key {
        float Value[4] = { };
    };
    /// anim curve types, the values are matching the NAX3 curve types (N3's CoreAnimation::CurveType)
    struct CurveType {
        enum Enum {
            Translation = 0,
            Scale,
            Rotation,
            Color,
            Float4,
            //--
            Num,
            Invalid,
        };

        static const char* ToString(Enum e) {
            switch (e) {
                case Translation: return "Translation";
                case Scale: return "Scale";
                case Rotation: return "Rotation";
                case Color: return "Color";
                case Float4: return "Float4";
                default: return "Invalid";
            }
        };
    };
    /// an animation curve is a container for animation keys
    struct Curve {
        bool IsStatic = false;
        bool IsActive = false;
        CurveType::Enum Type = CurveType::Invalid;
        Key StaticKey;
        std::vector<Key> Keys;
    };
    /// an animation clip hols animation curves
    struct Clip {
        std::string Name;
        float KeyDuration = 0.0f;   // in seconds!
        std::vector<Curve> Curves;
    };
    /// all the clips
    std::vector<Clip> Clips;

    // NAX3 file format structs and constants
    #pragma pack(push, 1)
    struct Nax3Header {
        uint32_t Magic;
        uint32_t NumClips;
        uint32_t NumKeys;
    };
    struct Nax3Clip {
        uint16_t NumCurves;
        uint16_t StartKeyIndex;
        uint16_t NumKeys;
        uint16_t KeyStride;
        uint16_t KeyDuration;
        uint8_t PreInfinityType;
        uint8_t PostInfinityType;
        uint16_t NumEvents;
        uint8_t Name[50];
    };
    struct Nax3AnimEvent {
        uint8_t Name[47];
        uint8_t Category[15];
        uint16_t KeyIndex;
    };
    struct Nax3Curve {
        uint32_t FirstKeyIndex;
        uint8_t IsActive;
        uint8_t IsStatic;
        uint8_t CurveType;
        uint8_t Padding;
        float StaticKey[4];
    };
    struct Nac3Header {
        uint32_t Magic;
    };
    #pragma pack(pop)
};
