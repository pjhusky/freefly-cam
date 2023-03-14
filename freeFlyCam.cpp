#include "freeFlyCam.h"

#include "../math/linAlg.h" // remove this dependency eventually

#ifndef _USE_MATH_DEFINES
    #define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <numbers>

namespace {
    constexpr static float practicallyZero = 0.00001f;
    constexpr static float mouseSensitivityInit = 0.23f;

    static void loadIdentityMatrix( FreeFlyCam::rowMajorMat3x4_t& mat ) {
        mat[0] = FreeFlyCam::rowVec4_t{ 1.0f, 0.0f, 0.0f, 0.0f };
        mat[1] = FreeFlyCam::rowVec4_t{ 0.0f, 1.0f, 0.0f, 0.0f };
        mat[2] = FreeFlyCam::rowVec4_t{ 0.0f, 0.0f, 1.0f, 0.0f };
    }
    
     //template<uint32_t num_T>
     //static FreeFlyCam::vec_t<num_T>& operator*( FreeFlyCam::vec_t<num_T>& vec, const float scalar ) {
     //    for ( auto& entry : vec ) {
     //        entry *= scalar;
     //    }
     //    return vec;
     //}
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

FreeFlyCam::FreeFlyCam() 
    : mLMBdown( false )
    , mRMBdown( false )
    , mIsActive( true ) {

    resetTrafos();

    setControlConfig( ControlConfig{ .invertY=1u } );

    setMouseSensitivity( mouseSensitivityInit );

    mCurrMouseX = 0.0f;
    mCurrMouseY = 0.0f;
    mPrevMouseX = mCurrMouseX;
    mPrevMouseY = mCurrMouseY;
}

FreeFlyCam::Status_t FreeFlyCam::update( 
    const float timeDelta,
    const float mouseX, const float mouseY, 
    const int32_t screenW, const int32_t screenH, 
    const bool LMBpressed, const bool RMBpressed, 
    const rowVec3_t& translationDelta
    ) {

    if (!mIsActive) { return Status_t::OK; }

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

    rowVec3_t *const xAxis = reinterpret_cast< rowVec3_t *const >( &mViewMat[0] );
    rowVec3_t *const yAxis = reinterpret_cast< rowVec3_t *const >( &mViewMat[1] );
    rowVec3_t *const zAxis = reinterpret_cast< rowVec3_t *const >( &mViewMat[2] );

    rowVec3_t currXAxis = *xAxis;
    rowVec3_t currYAxis = *yAxis;
    rowVec3_t currZAxis = *zAxis;

    if ( mLMBdown ) {

        {
            const float angleRad = static_cast<float>(std::numbers::pi) * mouse_dx / screenW;
            const float cosAngle = cosf( angleRad );
            const float sinAngle = sinf( angleRad );
            *xAxis = (currXAxis *  cosAngle) + (currZAxis * sinAngle);
            *zAxis = (currXAxis * -sinAngle) + (currZAxis * cosAngle);
            currXAxis = *xAxis;
            currZAxis = *zAxis;
        }

        {
            const float angleRad = static_cast<float>(std::numbers::pi) * mouse_dy / screenH;
            const float cosAngle = cosf( angleRad );
            const float sinAngle = sinf( angleRad );
            *yAxis = (currYAxis *  cosAngle) + (currZAxis * sinAngle);
            *zAxis = (currYAxis * -sinAngle) + (currZAxis * cosAngle);
            currYAxis = *yAxis;
            currZAxis = *zAxis;
        }

    } else if ( mRMBdown ) {

        const float angleRad = static_cast<float>( std::numbers::pi ) * -mouse_dx / screenW;
        const float cosAngle = cosf( angleRad );
        const float sinAngle = sinf( angleRad );
        *xAxis = ( currXAxis *  cosAngle ) + ( currYAxis * sinAngle );
        *yAxis = ( currXAxis * -sinAngle ) + ( currYAxis * cosAngle );
        currXAxis = *xAxis;
        currYAxis = *yAxis;
    }

    mPosWS = mPosWS + ( currXAxis * translationDelta[0] ) + ( currYAxis * translationDelta[1] ) + ( currZAxis * translationDelta[2] );

    mViewMat[0][3] = -dot( currXAxis, mPosWS ); 
    mViewMat[1][3] = -dot( currYAxis, mPosWS );
    mViewMat[2][3] = -dot( currZAxis, mPosWS );
    
    mPrevMouseX = mCurrMouseX;
    mPrevMouseY = mCurrMouseY;

    return Status_t::OK;
}

void FreeFlyCam::setViewMatrix( const rowMajorMat3x4_t& viewMatrix ) {
    resetTrafos();

    mViewMat = viewMatrix;
    //mViewMat[0] = mViewMat[0] * -1.0f;
    //mViewMat[1] = mViewMat[1] * -1.0f;
    //mViewMat[2] = mViewMat[2] * -1.0f;

    //mViewMat[0][3] = -mViewMat[0][3];
    //mViewMat[1][3] = -mViewMat[1][3];
    //mViewMat[2][3] = -mViewMat[2][3];
    //rowVec4_t col0{ viewMatrix[0][0],viewMatrix[1][0],viewMatrix[2][0], 0.0f };
    //rowVec4_t col1{ viewMatrix[0][1],viewMatrix[1][1],viewMatrix[2][1], 0.0f };
    //rowVec4_t col2{ viewMatrix[0][2],viewMatrix[1][2],viewMatrix[2][2], 0.0f };

    linAlg::mat4_t viewMat4;
    linAlg::castMatrix( viewMat4, viewMatrix );
    linAlg::mat4_t invViewMat4;
    //linAlg::transpose( invViewMat4, viewMat4 );
    linAlg::inverse( invViewMat4, viewMat4 );

    //linAlg::vec4_t camPosES{ 0.0f, 0.0f, 0.0f, 1.0f };
    //linAlg::vec4_t camPosWS = camPosES;
    //linAlg::applyTransformationToPoint( invViewMat4, &camPosWS, 1 );
    //linAlg::applyTransformationToPoint( viewMat4, &camPosWS, 1 );

    //mPosWS = { camPosWS[0], camPosWS[1], camPosWS[2] };
    //mPosWS = { -camPosWS[0], -camPosWS[1], -camPosWS[2] };
    //mPosWS = { -mViewMat[0][3], -mViewMat[1][3], -mViewMat[2][3] };
    
    //setPosition( {invViewMat4[0][3], invViewMat4[1][3], invViewMat4[2][3] } );
    mPosWS = {invViewMat4[0][3], invViewMat4[1][3], invViewMat4[2][3] };
}

void FreeFlyCam::setPosition( const rowVec3_t& pos ) {
    mPosWS = pos;

    rowVec3_t *const xAxis = reinterpret_cast< rowVec3_t *const >( &mViewMat[0] );
    rowVec3_t *const yAxis = reinterpret_cast< rowVec3_t *const >( &mViewMat[1] );
    rowVec3_t *const zAxis = reinterpret_cast< rowVec3_t *const >( &mViewMat[2] );

    rowVec3_t currXAxis = *xAxis;
    rowVec3_t currYAxis = *yAxis;
    rowVec3_t currZAxis = *zAxis;

    mViewMat[0][3] = -dot( currXAxis, mPosWS ); 
    mViewMat[1][3] = -dot( currYAxis, mPosWS );
    mViewMat[2][3] = -dot( currZAxis, mPosWS );
}

void FreeFlyCam::resetTrafos() {
    
    loadIdentityMatrix( mViewMat );

    mPosWS = { 0.0f, 0.0f, 0.0f };

    mCurrMouseX = 0;
    mCurrMouseY = 0;
    mPrevMouseX = 0;
    mPrevMouseY = 0;
    
}
