#include "Framework/LogicUnitHandler/LogicUnitDispatch.h"
#include "Framework/ScenesHandler/SceneLoader.h"


#include "Modules/TextureDictionary/TextureDictionary.h"
#include "Modules/UI/TexturedQuad.h"
#include "Modules/StaticMesh/StaticMesh.h"
#include "Modules/Camera/Camera.h"
#include "Modules/ControlInput/Controller.h"
#include "Modules/AreaTriggerBox/AreaTriggerBox.h"
#include "Modules/Audio/AudioSource.h"

namespace GV
{
    void LogicUnitDispatch::Dispatch(
        const std::vector<uint8_t>& bytes,
        const std::vector<SceneDispatchItem>& items)
    {
        for (const SceneDispatchItem& item : items)
        {
            switch (item.type)
            {
                case 0x0016: 
                    TextureDictionary::Load(bytes, item.start, item.end);
                    break;

                case 0x0013: 
                    StaticMesh::Load(bytes, item.start, item.end);
                    break;

                case 0x0005: 
                    Camera::Load(bytes, item.start, item.end);
                    break;
                    
                case 0x0006:
                    TexturedQuad::Load(bytes, item.start, item.end);
                    break;
                case 0x0014:
                    InputController::Load(bytes, item.start, item.end);
                    break;
                case 0x0012:
                    AreaTriggerBox::Load(bytes, item.start, item.end);
                    break;
                case 0x0035:
                    AudioSource::Load(bytes, item.start, item.end);
                    break;

                default:
                    break;
            }
        }
    }
}