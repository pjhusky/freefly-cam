#ifndef _FREEFLYCAM_H_9ec4f00a_2117_4578_937e_9f4fb94dc759
#define _FREEFLYCAM_H_9ec4f00a_2117_4578_937e_9f4fb94dc759

#include <array>
#include <stdint.h>

struct FreeFlyCam {

    enum class Status_t : int32_t {
        OK,
        ERROR,
    };

    struct ControlConfig {
        uint32_t invertY : 1;
    };
    
    template <std::size_t dimensionality_T>
    using vec_t = std::array< float, dimensionality_T >;
    
    using rowVec3_t = vec_t<3u>;
    using rowVec4_t = vec_t<4u>;
    using rowMajorMat3x4_t = std::array< rowVec4_t, 3u >;

    FreeFlyCam();
    
    Status_t update( const float timeDelta,
                     const float relativeMouseX, const float relativeMouseY, 
                     const bool LMBpressed, const bool RMBpressed, 
                     const rowVec3_t& translationDelta
                     );
    
    const rowMajorMat3x4_t& getViewMatrix() const { return mViewMat; }
    void setViewMatrix( const rowMajorMat3x4_t& viewMatrix );

    void setPosition( const rowVec3_t& posWS );
    void lookAt( const rowVec3_t& lookAtPosWS );

    void setControlConfig( const ControlConfig controlConfig ) { mControlConfig = controlConfig; }

    void setMouseSensitivity( const float mouseSensitivity ) { mMouseSensitivity = mouseSensitivity; }
    void setActive( const bool isActive ) { mIsActive = isActive; }

    void resetTrafos();

private:

    rowMajorMat3x4_t mViewMat;
    rowVec3_t mPosWS;
    
    ControlConfig mControlConfig;
    
    float mRelativeCurrMouseX;
    float mRelativeCurrMouseY;
    float mPrevRelativeMouseX;
    float mPrevRelativeMouseY;
    float mMouseSensitivity;
    
    bool  mLMBdown;
    bool  mRMBdown;

    bool  mIsActive;
};

#endif // _FREEFLYCAM_H_9ec4f00a_2117_4578_937e_9f4fb94dc759
