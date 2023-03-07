#include "FreeFlyCam.h"

#ifndef _USE_MATH_DEFINES
    #define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <numbers>

namespace {
    constexpr static float practicallyZero = 0.00001f;
    
    // constexpr static float dampingFactorInit = 0.91f;
    constexpr static float mouseSensitivityInit = 0.23f;

    static void loadIdentityMatrix( FreeFlyCam::rowMajorMat3x4_t& mat ) {
        //for ( uint32_t row = 0; row < mat.size(); row++ ) {
        //}
        mat[0] = FreeFlyCam::rowVec4_t{ 1.0f, 0.0f, 0.0f, 0.0f };
        mat[1] = FreeFlyCam::rowVec4_t{ 0.0f, 1.0f, 0.0f, 0.0f };
        mat[2] = FreeFlyCam::rowVec4_t{ 0.0f, 0.0f, 1.0f, 0.0f };
    }
    
    // template<uint32_t num_T>
    // static FreeFlyCam::vec_t<num_T>& operator*( FreeFlyCam::vec_t<num_T>& vec, const float scalar ) {
    //     for ( auto& entry : vec ) {
    //         entry *= scalar;
    //     }
    //     return vec;
    // }
    template<std::size_t num_T>
    static inline FreeFlyCam::vec_t<num_T> operator*( const FreeFlyCam::vec_t<num_T>& vec, const float scalar ) {
        FreeFlyCam::vec_t<num_T> result = vec;
        for ( auto& entry : result ) {
            entry *= scalar;
        }
        return result;
    }

    template<std::size_t num_T>
    static inline FreeFlyCam::vec_t<num_T> operator+( const FreeFlyCam::vec_t<num_T>& vecL, const FreeFlyCam::vec_t<num_T>& vecR ) {
        FreeFlyCam::vec_t<num_T> result = vecL;
        for ( uint32_t i = 0; i < num_T; i++ ) {
            result[i] += vecR[i];
        }
        return result;
    }
    
    template<std::size_t num_T>
    static inline float dot( const FreeFlyCam::vec_t<num_T>& vecL, const FreeFlyCam::vec_t<num_T>& vecR ) {
        float result = 0.0f;
        for ( uint32_t i = 0; i < num_T; i++ ) {
            result += vecL[i] * vecR[i];
        }
        return result;
    }
    
}

FreeFlyCam::FreeFlyCam() {

    resetTrafos();

    setControlConfig( ControlConfig{ .invertY=1u } );

    // setDampingFactor( dampingFactorInit );
    setMouseSensitivity( mouseSensitivityInit );
    mLMBdown = false;
    mRMBdown = false;

    mCurrMouseX = 0.0f;
    mCurrMouseY = 0.0f;
    mPrevMouseX = mCurrMouseX;
    mPrevMouseY = mCurrMouseY;
    
//     mTargetMouse_dx = 0.0f;
//     mTargetMouse_dy = 0.0f;

    // mStartMouseNDC = linAlg::vec3_t{ 0.0f, 0.0f, 0.0f };
    // mCurrMouseNDC = linAlg::vec3_t{ 0.0f, 0.0f, 0.0f };
}

FreeFlyCam::Status_t FreeFlyCam::update( 
    const float timeDelta,
    const float mouseX, const float mouseY, 
    const int32_t screenW, const int32_t screenH, 
    const bool LMBpressed, const bool RMBpressed, 
    const rowVec3_t& translationDelta
    ) {

    mCurrMouseX = mouseX;
    mCurrMouseY = mouseY;
    const float mouse_dx = (mCurrMouseX - mPrevMouseX) * mMouseSensitivity;
    const float mouse_dy = (mCurrMouseY - mPrevMouseY) * mMouseSensitivity;

    if (!mLMBdown && LMBpressed) {
        printf( "LMB pressed\n" );
        mLMBdown = true;
    }
    if (mLMBdown && !LMBpressed) {
        printf( "LMB released\n" );
        mLMBdown = false;
    }

    if (!mRMBdown && RMBpressed) {
        printf( "RMB pressed\n" );
        mRMBdown = true;
    }
    if (mRMBdown && !RMBpressed) {
        printf( "RMB released\n" );
        mRMBdown = false;
    }

    // if (mLMBdown) {
    //     mTargetMouse_dx += mouse_dx;
    //     mTargetMouse_dy += mouse_dy;
    // }

    // if ((mTargetMouse_dx * mTargetMouse_dx + mTargetMouse_dy * mTargetMouse_dy > 0.001) || mLMBdown || mRMBdown ){ // 
    //     if ( mLMBdown ) {            
    //     }
    // }

    rowVec3_t *const xAxis = reinterpret_cast< rowVec3_t *const >( &mViewMat[0] );
    rowVec3_t *const yAxis = reinterpret_cast< rowVec3_t *const >( &mViewMat[1] );
    rowVec3_t *const zAxis = reinterpret_cast< rowVec3_t *const >( &mViewMat[2] );

    /* const */ rowVec3_t currXAxis = *xAxis;
    /* const */ rowVec3_t currYAxis = *yAxis;
    /* const */ rowVec3_t currZAxis = *zAxis;



    if ( mLMBdown ) {

        {
            const float angleRad = static_cast<float>(std::numbers::pi) * mouse_dx / screenW;
            const float cosAngle = cosf( angleRad );
            const float sinAngle = sinf( angleRad );
            *xAxis = (currXAxis *  cosAngle) + (currZAxis * sinAngle);
            *zAxis = (currXAxis * -sinAngle) + (currZAxis * cosAngle);

            currXAxis = *xAxis;
            currYAxis = *yAxis;
            currZAxis = *zAxis;
        }

        {
            const float angleRad = static_cast<float>(std::numbers::pi) * mouse_dy / screenH;
            const float cosAngle = cosf( angleRad );
            const float sinAngle = sinf( angleRad );
            *yAxis = (currYAxis *  cosAngle) + (currZAxis * sinAngle);
            *zAxis = (currYAxis * -sinAngle) + (currZAxis * cosAngle);
            currXAxis = *xAxis;
            currYAxis = *yAxis;
            currZAxis = *zAxis;
        }

    } else if ( mRMBdown ) {

        const float angleRad = static_cast<float>( std::numbers::pi ) * -mouse_dx / screenW;
        const float cosAngle = cosf( angleRad );
        const float sinAngle = sinf( angleRad );
        //const float sinAngle = mouse_dy / screenH;
        //*xAxis = add( mul( oldXAxis,  cosAngle ), mul( oldYAxis, sinAngle ) );
        *xAxis = ( currXAxis *  cosAngle ) + ( currYAxis * sinAngle );
        *yAxis = ( currXAxis * -sinAngle ) + ( currYAxis * cosAngle );
        currXAxis = *xAxis;
        currYAxis = *yAxis;
        currZAxis = *zAxis;
    }



    //mPosWS = mPosWS + translationDelta;
    mPosWS = mPosWS + ( currXAxis * translationDelta[0] ) + ( currYAxis * translationDelta[1] ) + ( currZAxis * translationDelta[2] );

    mViewMat[0][3] = -dot( currXAxis, mPosWS ); 
    mViewMat[1][3] = -dot( currYAxis, mPosWS );
    mViewMat[2][3] = -dot( currZAxis, mPosWS );
    
    

    
    
    mPrevMouseX = mCurrMouseX;
    mPrevMouseY = mCurrMouseY;

    // if (fabsf( mTargetMouse_dx ) > practicallyZero) { // prevent mTargetmouse_dx from becomming too small "#DEN => denormalized" - may have caused the weird disappearance glitch on mouse interaction
    //     mTargetMouse_dx *= 0.91f;
    // }
    // if (fabsf( mTargetMouse_dy ) > practicallyZero) { // prevent mTargetmouse_dx from becomming too small "#DEN => denormalized" - may have caused the weird disappearance glitch on mouse interaction
    //     mTargetMouse_dy *= 0.91f;
    // }
    //}

    return Status_t::OK;
}

void FreeFlyCam::resetTrafos() {
    //linAlg::loadIdentityMatrix( mViewMat );
    loadIdentityMatrix( mViewMat );

    mPosWS = { 0.0f, 0.0f, 0.0f };

    // linAlg::loadIdentityMatrix( mCurrRotMat );
    // linAlg::loadIdentityMatrix( mPrevRotMat );

    // mStartMouseNDC = linAlg::vec3_t{ 0.0f, 0.0f, 1.0f };
    // mCurrMouseNDC  = linAlg::vec3_t{ 0.0f, 0.0f, 1.0f };

    mCurrMouseX = 0;
    mCurrMouseY = 0;
    mPrevMouseX = 0;
    mPrevMouseY = 0;
    // mTargetMouse_dx = 0;
    // mTargetMouse_dy = 0;

}
