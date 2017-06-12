//------------------------------------------------------------------------------
//  NAX3Loader.cc
//------------------------------------------------------------------------------
#include "NAX3Loader.h"
#include "ExportUtil/Log.h"
#include "LoadUtil.h"
#include "pystring.h"

using namespace OryolTools;

//------------------------------------------------------------------------------
void
NAX3Loader::Clear() {
    this->Clips.clear();
}

//------------------------------------------------------------------------------
void
NAX3Loader::Load(const std::string& nax3AssetName, const std::string& n3AssetDir) {
    if (!this->Clips.empty()) {
        this->Clear();
    }

    // load the entire file into memory
    std::string path = n3AssetDir + "/anims/" + nax3AssetName;
    const uint8_t* start = load_file(path);

    // parse the header
    const uint8_t* ptr = start;
    const Nax3Header* nax3Hdr = (const Nax3Header*) ptr;
    Log::FailIf(nax3Hdr->Magic != 'NAH0', "Magic number mismatch for '%s'\n", path.c_str());
    ptr += sizeof(Nax3Header);

    // read clips
    this->Clips.reserve(nax3Hdr->NumClips);
    for (int clipIndex = 0; clipIndex < int(nax3Hdr->NumClips); clipIndex++) {
        const Nax3Clip* nax3Clip = (const Nax3Clip*) ptr;
        ptr += sizeof(Nax3Clip);
        this->Clips.push_back(Clip());
        auto& clip = this->Clips.back();
        clip.Name = (const char*) nax3Clip->Name;
        clip.KeyDuration = float(nax3Clip->KeyDuration) / 1000.0f;
        clip.Curves.reserve(nax3Clip->NumCurves);
        
        // skip anim events
        ptr += nax3Clip->NumEvents * sizeof(Nax3AnimEvent);

        // read curve info
        const Nax3Curve* nax3Curves = (const Nax3Curve*) ptr;
        bool clipHasKeyData = false;
        for (int curveIndex = 0; curveIndex < nax3Clip->NumCurves; curveIndex++) {
            const Nax3Curve* nax3Curve = &nax3Curves[curveIndex];
            clip.Curves.push_back(Curve());
            auto& curve = clip.Curves.back();
            curve.IsStatic = nax3Curve->IsStatic;
            curve.IsActive = nax3Curve->IsActive;
            curve.Type = (CurveType::Enum) nax3Curve->CurveType;
            for (int i = 0; i < 4; i++) {
                curve.StaticKey[i] = nax3Curve->StaticKey[i];
            }
            if (!curve.IsStatic && curve.IsActive) {
                curve.Keys.reserve(nax3Clip->NumKeys);
                clipHasKeyData = true;
            }
        }
        ptr += sizeof(Nax3Curve) * nax3Clip->NumCurves;

        // load anim keys from separate clip file
        if (clipHasKeyData) {
            std::string nacPath = n3AssetDir + "/anims/";
            nacPath += pystring::replace(nax3AssetName, "_animations.nax3", "");
            nacPath += "_" + clip.Name + ".nac";
            const uint8_t* nacStart = load_file(nacPath);
            const uint8_t* nacPtr = nacStart;
            const Nac3Header* nac3Hdr = (const Nac3Header*) nacPtr;
            Log::FailIf(nac3Hdr->Magic != 'NAC0', "Magic number mismatch for clip file '%s'\n", nacPath.c_str());
            nacPtr += sizeof(Nac3Header);
            const int numCurves = nax3Clip->NumCurves;
            const int numKeys = nax3Clip->NumKeys;
            for (int keyIndex = 0; keyIndex < numKeys; keyIndex++) {
                for (int curveIndex = 0; curveIndex < numCurves; curveIndex++) {
                    const Nax3Curve* nax3Curve = &nax3Curves[curveIndex];
                    if (!nax3Curve->IsStatic && nax3Curve->IsActive) {
                        glm::vec4 dstKey;
                        if (nax3Curve->CurveType == CurveType::Rotation) {
                            // this is a packed quaternion with 16-bit components
                            const float div = (const float) 0x7FFF;
                            for (int i = 0; i < 4; i++) {
                                dstKey[i] = (float)((int16_t*)nacPtr)[i] / div;
                            }
                            nacPtr += 8;
                        }
                        else {
                            // unpacked, 4 float components
                            for (int i = 0; i < 4; i++) {
                                dstKey[i] = ((float*)nacPtr)[i];
                            }
                        }
                        clip.Curves[curveIndex].Keys.push_back(dstKey);
                    }
                }
            }
            free_file_data(nacStart);
        }
    }
    free_file_data(start);
}

//------------------------------------------------------------------------------
void
NAX3Loader::Validate() {
    // must have at least one clip
    Log::FailIf(this->Clips.empty(), "No clips in animation found!\n");

    // all clips must have same number of curves
    const int numCurves = this->Clips[0].Curves.size();
    for (const auto& clip : this->Clips) {
        Log::FailIf(numCurves != int(clip.Curves.size()), "Inconsistent number of anim curves!\n");
    }

    // all clips must have the same 'curve layout'
    const auto& clip0 = this->Clips[0];
    for (const auto& clip : this->Clips) {
        for (int i = 0; i < numCurves; i++) {
            Log::FailIf(clip0.Curves[i].Type != clip.Curves[i].Type, "Inconsistent anim curve layout!\n");
        }
    }

    // all curves of a clip must either have no keys, or the same number of keys
    for (const auto& clip : this->Clips) {
        int numKeys = 0;
        for (const auto& curve : clip.Curves) {
            if ((numKeys == 0) && !curve.Keys.empty()) {
                numKeys = curve.Keys.size();
            }
            if (!curve.Keys.empty()) {
                Log::FailIf(int(curve.Keys.size()) != numKeys, "Inconsistent number of keys in curves!\n");
            }
        }
    }
}
