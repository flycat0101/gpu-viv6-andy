/****************************************************************************
*
*    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


/*
 * Mesa 3-D graphics library
 * Version:  3.5
 *
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define _CRT_SECURE_NO_WARNINGS
#ifdef UNDER_CE
#include <windows.h>
#endif
#include "vdk_sample_common.h"

void VDKS_Func_GetCurrentDir(char path[MAX_PATH + 1])
{
#ifdef _WIN32
    char* pos;
#if defined(UNICODE) || defined(_UNICODE)
    static WCHAR wpath[MAX_PATH + 1];
    GetModuleFileName(NULL, wpath, MAX_PATH);
    wcstombs(path, wpath, MAX_PATH);
#else
    GetModuleFileName(NULL, path, MAX_PATH);
#endif
    pos = strrchr(path, '\\');
    *(pos + 1) = '\0';
#else /* Linux */
    strcpy(path, "./");
#endif
}

int VDKS_Func_AlertUser(int StopRunning, char* FormatString, ...)
{
    va_list VarList;

    va_start(VarList, FormatString);

    vfprintf(stderr, FormatString, VarList);

    va_end(VarList);

    if (StopRunning)
    {
        exit(-1);
    }

    return 0;
}

VDKS_BOOL VDKS_Func_SaveScreen(char* Name)
{
    unsigned int size = sizeof(unsigned char) * VDKS_Val_WindowsWidth * VDKS_Val_WindowsHeight * 4;
    unsigned char * rgba = (unsigned char *)malloc(size);
    memset(rgba, 0, size);
    if (NULL == rgba) {
        printf("VDKS_Func_SaveScreen : out-of-memory\n");
        return VDKS_FALSE;
    }

    assert(rgba);

    glReadPixels(
        0, 0,
        VDKS_Val_WindowsWidth, VDKS_Val_WindowsHeight,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        rgba);

    /*
    *    This will change "rgba'.
    */
    VDKS_Func_SaveBmp(Name, VDKS_Val_WindowsWidth, VDKS_Val_WindowsHeight, rgba);

    free(rgba);

    return VDKS_TRUE;
}

/*
 * Matrix Operations
*/

static float _VDKS_Identity [16] =
{
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};

void VDKS_Func_LoadIdentity ( float *mat )
{
    memcpy(mat, _VDKS_Identity, sizeof(_VDKS_Identity));
    return;
}

void VDKS_Func_LookAt( float view[3], float focus[3], float up[3], float matrix[16] )
{
    float *m = matrix;
    float x[3], y[3], z[3];
    float mag;

    /* Rotation Matrix*/
    /* Z axis. */
    z[0] = view[0] - focus[0];
    z[1] = view[1] - focus[1];
    z[2] = view[2] - focus[2];
    mag = (float)sqrt( z[0] * z[0] + z[1] * z[1] + z[2] * z[2] );
    z[0] /= mag;
    z[1] /= mag;
    z[2] /= mag;

    /* Temp Y axis. */
    y[0] = up[0];
    y[1] = up[1];
    y[2] = up[2];

    /* X axis = Y axis cross Z axis. */
    x[0] = y[1] * z[2] - y[2] * z[1];
    x[1] = y[2] * z[0] - y[0] * z[2];
    x[2] = y[0] * z[1] - y[1] * z[0];

    /* Again, compute the final Y. Y = Z x X. */
    y[0] = z[1] * x[2] - z[2] * x[1];
    y[1] = z[2] * x[0] - z[0] * x[2];
    y[2] = z[0] * x[1] - z[1] * x[0];

    /* Normalize X and Y. */
    mag = (float)sqrt( x[0] * x[0] + x[1] * x[1] + z[2] * x[2] );
    x[0] /= mag;
    x[1] /= mag;
    x[2] /= mag;

    mag = (float)sqrt( y[0] * y[0] + y[1] * y[1] + y[2] * y[2] );
    y[0] /= mag;
    y[1] /= mag;
    y[2] /= mag;

    /* Calculate the matrix. */
    m[0] = x[0], m[4] = x[1], m[8]  = x[2], m[12] = -view[0] * x[0] - view[1] * x[1] - view[2] * x[2];
    m[1] = y[0], m[5] = y[1], m[9]  = y[2], m[13] = -view[0] * y[0] - view[1] * y[1] - view[2] * y[2];
    m[2] = z[0], m[6] = z[1], m[10] = z[2], m[14] = -view[0] * z[0] - view[1] * z[1] - view[2] * z[2];
    m[3] = 0.0f, m[7] = 0.0f, m[11] = 0.0f, m[15] = 1.0f;
}

/*
*    Known BUG : Do not set view[2] = 0.0f;
*/
void VDKS_Func_Matrix_LookAt( float view[3], float focus[3], float up[3], float matrix[16] )
{
    float src [16];

    float lookat [16];

    VDKS_Func_LoadIdentity(lookat);

    VDKS_Func_LookAt(view, focus, up, lookat);

    memcpy(src, matrix, sizeof(float) * 16);

    VDKS_Func_Matrix_Mul4by4(matrix, lookat, src);
}


void VDKS_Func_Ortho(  float left, float right, float bottom, float top, float nearval, float farval,   float *m )
{
#define M(row,col)  m[col*4+row]
    M(0,0) = 2.0F / (right-left);
    M(0,1) = 0.0F;
    M(0,2) = 0.0F;
    M(0,3) = -(right+left) / (right-left);

    M(1,0) = 0.0F;
    M(1,1) = 2.0F / (top-bottom);
    M(1,2) = 0.0F;
    M(1,3) = -(top+bottom) / (top-bottom);

    M(2,0) = 0.0F;
    M(2,1) = 0.0F;
    M(2,2) = -2.0F / (farval-nearval);
    M(2,3) = -(farval+nearval) / (farval-nearval);

    M(3,0) = 0.0F;
    M(3,1) = 0.0F;
    M(3,2) = 0.0F;
    M(3,3) = 1.0F;
#undef M
}

void VDKS_Func_Matrix_Ortho(  float left, float right, float bottom, float top, float nearval, float farval,   float *m )
{
    float src [16];
    float ortho [16];

    VDKS_Func_LoadIdentity(ortho);

    VDKS_Func_Ortho(left,right, bottom, top, nearval, farval, ortho);

    memcpy(src, m, sizeof(float) * 16);

    VDKS_Func_Matrix_Mul4by4(m, ortho, src);
}


#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]

void    VDKS_Func_Matrix_Mul4by4( float *product, const float *a, const float *b )
{
    int i;

    if(product == b)
    {
        printf("Error in MatrixMul4by4, product == b\n");
        return;
    }

    for (i = 0; i < 4; i++) {
        const float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
        P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
        P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
        P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
        P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
    }
}

#undef A
#undef B
#undef P

#ifndef M_PI
#define M_PI (3.1415926536)
#endif

#define DEG2RAD (M_PI/180.0)
void VDKS_Func_Rotate( float *mat, float angle, float x, float y, float z )
{
    float xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c, s, c;
    float m[16];
    VDKS_BOOL optimized;

    s = (float) sin( angle * DEG2RAD );
    c = (float) cos( angle * DEG2RAD );

    memcpy(m, _VDKS_Identity, sizeof(float)*16);
    optimized = VDKS_FALSE;

#define M(row,col)  m[col*4+row]

    if (x == 0.0F) {
        if (y == 0.0F) {
            if (z != 0.0F) {
                optimized = VDKS_TRUE;
                /* rotate only around z-axis */
                M(0,0) = c;
                M(1,1) = c;
                if (z < 0.0F) {
                    M(0,1) = s;
                    M(1,0) = -s;
                }
                else {
                    M(0,1) = -s;
                    M(1,0) = s;
                }
            }
        }
        else if (z == 0.0F) {
            optimized = VDKS_TRUE;
            /* rotate only around y-axis */
            M(0,0) = c;
            M(2,2) = c;
            if (y < 0.0F) {
                M(0,2) = -s;
                M(2,0) = s;
            }
            else {
                M(0,2) = s;
                M(2,0) = -s;
            }
        }
    }
    else if (y == 0.0F) {
        if (z == 0.0F) {
            optimized = VDKS_TRUE;
            /* rotate only around x-axis */
            M(1,1) = c;
            M(2,2) = c;
            if (x < 0.0F) {
                M(1,2) = s;
                M(2,1) = -s;
            }
            else {
                M(1,2) = -s;
                M(2,1) = s;
            }
        }
    }

    if (!optimized) {
        const float mag = (float)sqrt(x * x + y * y + z * z);

        if (mag <= 1.0e-4) {
            /* no rotation, leave mat as-is */
            return;
        }

        x /= mag;
        y /= mag;
        z /= mag;


        /*
        *     Arbitrary axis rotation matrix.
        *
        *  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
        *  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
        *  (which is about the X-axis), and the two composite transforms
        *  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
        *  from the arbitrary axis to the X-axis then back.  They are
        *  all elementary rotations.
        *
        *  Rz' is a rotation about the Z-axis, to bring the axis vector
        *  into the x-z plane.  Then Ry' is applied, rotating about the
        *  Y-axis to bring the axis vector parallel with the X-axis.  The
        *  rotation about the X-axis is then performed.  Ry and Rz are
        *  simply the respective inverse transforms to bring the arbitrary
        *  axis back to it's original orientation.  The first transforms
        *  Rz' and Ry' are considered inverses, since the data from the
        *  arbitrary axis gives you info on how to get to it, not how
        *  to get away from it, and an inverse must be applied.
        *
        *  The basic calculation used is to recognize that the arbitrary
        *  axis vector (x, y, z), since it is of unit length, actually
        *  represents the sines and cosines of the angles to rotate the
        *  X-axis to the same orientation, with theta being the angle about
        *  Z and phi the angle about Y (in the order described above)
        *  as follows:
        *
        *  cos ( theta ) = x / sqrt ( 1 - z^2 )
        *  sin ( theta ) = y / sqrt ( 1 - z^2 )
        *
        *  cos ( phi ) = sqrt ( 1 - z^2 )
        *  sin ( phi ) = z
        *
        *  Note that cos ( phi ) can further be inserted to the above
        *  formulas:
        *
        *  cos ( theta ) = x / cos ( phi )
        *  sin ( theta ) = y / sin ( phi )
        *
        *  ...etc.  Because of those relations and the standard trigonometric
        *  relations, it is pssible to reduce the transforms down to what
        *  is used below.  It may be that any primary axis chosen will give the
        *  same results (modulo a sign convention) using thie method.
        *
        *  Particularly nice is to notice that all divisions that might
        *  have caused trouble when parallel to certain planes or
        *  axis go away with care paid to reducing the expressions.
        *  After checking, it does perform correctly under all cases, since
        *  in all the cases of division where the denominator would have
        *  been zero, the numerator would have been zero as well, giving
        *  the expected result.
        */

        xx = x * x;
        yy = y * y;
        zz = z * z;
        xy = x * y;
        yz = y * z;
        zx = z * x;
        xs = x * s;
        ys = y * s;
        zs = z * s;
        one_c = 1.0F - c;


        M(0,0) = (one_c * xx) + c;
        M(0,1) = (one_c * xy) - zs;
        M(0,2) = (one_c * zx) + ys;
        M(0,3) = 0.0F;

        M(1,0) = (one_c * xy) + zs;
        M(1,1) = (one_c * yy) + c;
        M(1,2) = (one_c * yz) - xs;
        M(1,3) = 0.0F;

        M(2,0) = (one_c * zx) - ys;
        M(2,1) = (one_c * yz) + xs;
        M(2,2) = (one_c * zz) + c;
        M(2,3) = 0.0F;


        M(3,0) = 0.0F;
        M(3,1) = 0.0F;
        M(3,2) = 0.0F;
        M(3,3) = 1.0F;

    }
#undef M

    VDKS_Func_Matrix_Mul4by4( mat, mat, m );
}
#undef DEG2RAD
#undef M_PI

void VDKS_Func_Matrix_Rotate( float *mat, float angle, float x, float y, float z )
{
    float rotate [16];

    float src [16];

    VDKS_Func_LoadIdentity(rotate);

    VDKS_Func_Rotate(rotate, angle, x, y, z);

    memcpy(src, mat, 16 * sizeof(float));

    VDKS_Func_Matrix_Mul4by4(mat, rotate, src);
}

void VDKS_Func_Matrix_Frustum    (float * left_mat, float left, float right, float bottom, float top, float nearval, float farval)
{
    float frustum [16];

    float src [16];

    VDKS_Func_LoadIdentity(frustum);

    VDKS_Func_Frustum (left, right, bottom, top, nearval, farval, frustum);

    memcpy(src, left_mat, 16 * sizeof(float));

    VDKS_Func_Matrix_Mul4by4(left_mat, frustum, src);

}

void VDKS_Func_Frustum                (float left, float right, float bottom, float top, float nearval, float farval, float* m)
{
    float x, y, a, b, c, d;

    x = (2.0F*nearval) / (right-left);
    y = (2.0F*nearval) / (top-bottom);
    a = (right+left) / (right-left);
    b = (top+bottom) / (top-bottom);
    c = -(farval+nearval) / ( farval-nearval);
    d = -(2.0F*farval*nearval) / (farval-nearval);  /* error? */

#define M(row,col)  m[col*4+row]
    M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
    M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
    M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
    M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M
}

void VDKS_Func_Translate(float *mat, float x, float y, float z )
{
    float *m = mat;

    m[12] = m[0] * x + m[4] * y + m[8]  * z + m[12];
    m[13] = m[1] * x + m[5] * y + m[9]  * z + m[13];
    m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
    m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];
}

void VDKS_Func_Matrix_Translate(float *mat, float x, float y, float z )
{
    float src [16];

    float translate [16];

    VDKS_Func_LoadIdentity(translate);

    VDKS_Func_Translate(translate, x, y, z);

    memcpy(src, mat, sizeof(float) * 16);

    VDKS_Func_Matrix_Mul4by4(mat, translate, src);
}

void VDKS_Func_Matrix_Transpose(float *mat)
{
    int w;
    int h;

    float tmp [16];

    memcpy(tmp, mat, sizeof(float) * 16);

    for(w = 0; w < 4 ; w++)
    {
        for(h = 0; h < 4 ; h++)
        {
            mat[w * 4 + h] = tmp [h * 4 + w];
        }
    }
}

/*
** From Mesa.
** inverse = invert(src)
** New, faster implementation by Shan Hao Bo, April 2006.
*/

int VDKS_Func_Matrix_Inverse(const float src[16], float inverse[16])
{
    int i, j, k;

    float t;

    float temp[4][4];

    for (i=0; i<4; i++)
    {
        for (j=0; j<4; j++)
        {
            temp[i][j] = src[i*4+j];
        }
    }

    VDKS_Func_LoadIdentity(inverse);

    for (i = 0; i < 4; i++) {
        if (temp[i][i] == 0.0f) {
            /*
            ** Look for non-zero element in column
            */
            for (j = i + 1; j < 4; j++) {
                if (temp[j][i] != 0.0f) {
                    break;
                }
            }

            if (j != 4) {
                /*
                 ** Swap rows.
                 */
                for (k = 0; k < 4; k++) {
                    t = temp[i][k];
                    temp[i][k] = temp[j][k];
                    temp[j][k] = t;

                    t = inverse[i*4+k];
                    inverse[i*4+k] = inverse[j*4+k];
                    inverse[j*4+k] = t;
                }
            }
            else {
                /*
                ** No non-zero pivot.  The matrix is singular, which shouldn't
                ** happen.  This means the user gave us a bad matrix.
                */
                return GL_FALSE;
            }
        }

        t = 1.0f / temp[i][i];
        for (k = 0; k < 4; k++) {
            temp[i][k] *= t;
            inverse[i*4+k] *= t;
        }
        for (j = 0; j < 4; j++) {
            if (j != i) {
                t = temp[j][i];
                for (k = 0; k < 4; k++) {
                        temp[j][k] -= temp[i][k]*t;
                        inverse[j*4+k] -= inverse[i*4+k]*t;
                }
            }
        }
    }
    return GL_TRUE;
}


/*
 * Shader & Program
*/

VDKS_BOOL VDKS_Func_BufferFile(const char* file, char** buffer, int * size)
{
    FILE *fp;
    char* buf= NULL;
    int file_size = 0;

    fp = fopen( file, "rb" );
    if (NULL == fp) {
        printf( "File could not be opened : %s\n",  file);
        return VDKS_FALSE;
    }

    if (-1 == fseek(fp, 0, SEEK_END)) goto onError;

    file_size = ftell(fp);

    buf = (char*)malloc( file_size + 1);
    if (NULL == buf) {
        printf("VDKS_Func_BufferFile : out-of-memory\n");
        goto onError;
    }

    if(-1 == fseek(fp, 0, SEEK_SET)) goto onError;
#ifndef ANDROID
    if (file_size != fread(buf, 1, file_size, fp)) goto onError;
#else
    if (file_size != (int)fread(buf, 1, file_size, fp)) goto onError;
#endif
    buf[file_size] = '\0';

    *buffer = buf;

    *size = file_size;

    fclose(fp);
    return VDKS_TRUE;

onError:
    fclose(fp);
    return VDKS_FALSE;
}

VDKS_BOOL VDKS_Func_CompileShaderFile(const char* file, GLuint shader)
{

    char    *buf = NULL;

    GLint     size = 0;

    GLint     error = 0;

    if (!VDKS_Func_BufferFile(file, &buf, &size))
    {
        return VDKS_FALSE;
    }
#ifndef ANDROID
    glShaderSource(shader, 1, (const GLubyte **)&buf, &size);
#else
    glShaderSource(shader, 1, (const char **)&buf, &size);
#endif
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &error);

    if (error == 0)
    {
        // Retrieve error buffer size.
        char * infoLog;
        GLint errorBufSize, errorLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &errorBufSize);

        infoLog = (char*)malloc(errorBufSize * sizeof (char) + 1);
        if (NULL != infoLog)
        {
            // Retrieve error.
            glGetShaderInfoLog(shader, errorBufSize, &errorLength, infoLog);
            infoLog[errorBufSize + 1] = '\0';
            printf("Failed to compile shader\n");
            printf("LOG: %s", infoLog);

            free(infoLog);
        }

        free(buf);
        return VDKS_FALSE;
    }

    free(buf);

    return VDKS_TRUE;
}


GLuint VDKS_Func_MakeShaderProgram(const char* vs_file, const char* fs_file)
{
    GLint error_code = 0;

    GLuint pro = 0;
    GLuint vs = 0;
    GLuint fs = 0;

    pro = glCreateProgram();

    vs = glCreateShader( GL_VERTEX_SHADER );

    if (!VDKS_Func_CompileShaderFile( vs_file, vs ))
        return 0;

    glAttachShader( pro , vs );

    fs = glCreateShader( GL_FRAGMENT_SHADER );

    if (!VDKS_Func_CompileShaderFile( fs_file, fs ))
        return 0;

    glAttachShader( pro , fs );

    glLinkProgram( pro );

    glGetProgramiv( pro, GL_LINK_STATUS, &error_code );

    if (!error_code)
    {
        // Retrieve error buffer size.
        char *infoLog;
        GLint errorBufSize, errorLength;
        glGetShaderiv(pro, GL_INFO_LOG_LENGTH, &errorBufSize);

        infoLog = (char*)malloc(errorBufSize * sizeof (char) + 1);
        if (NULL != infoLog)
        {
            // Retrieve error.
            glGetProgramInfoLog(pro, errorBufSize, &errorLength, infoLog);
            infoLog[errorBufSize + 1] = '\0';
            printf("Failed to link program object.");
            printf("LOG: %s", infoLog);

            free(infoLog);
        }

        return 0;
    }

    return pro;
}

GLuint VDKS_Func_MakeShaderProgram2(const char* vs_file, const char* fs_file, GLuint pro)
{
    GLint error_code = 0;

    GLuint vs = 0;
    GLuint fs = 0;

    vs = glCreateShader( GL_VERTEX_SHADER );

    if (!VDKS_Func_CompileShaderFile( vs_file, vs ))
        return 0;

    glAttachShader( pro , vs );

    fs = glCreateShader( GL_FRAGMENT_SHADER );

    if (!VDKS_Func_CompileShaderFile( fs_file, fs ))
        return 0;

    glAttachShader( pro , fs );

    glLinkProgram(pro);

    glGetProgramiv( pro, GL_LINK_STATUS, &error_code );

    if( error_code == GL_FALSE )
    {
        // Retrieve error buffer size.
        char *infoLog;
        GLint errorBufSize, errorLength;
        glGetShaderiv(pro, GL_INFO_LOG_LENGTH, &errorBufSize);

        infoLog = (char*)malloc(errorBufSize * sizeof (char) + 1);
        if (NULL  != infoLog)
        {
            // Retrieve error.
            glGetProgramInfoLog(pro, errorBufSize, &errorLength, infoLog);
            infoLog[errorBufSize + 1] = '\0';
            printf("Failed to link program object.");
            printf("LOG: %s", infoLog);

            free(infoLog);
        }

        return 0;
    }

    return pro;
}
#ifndef ANDROID
/*
 * EGL
*/
VDKS_BOOL VDKS_Init(vdkEGL * vdk_egl)
{
    EGLint configAttribs[] =
    {
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        EGL_ALPHA_SIZE,     8,
        EGL_DEPTH_SIZE,     24,
        EGL_STENCIL_SIZE,   EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS, EGL_DONT_CARE,
        EGL_SAMPLES,        EGL_DONT_CARE,
        EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
        EGL_NONE
    };

    EGLint ctxAttribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    memset(vdk_egl, 0, sizeof(vdkEGL));

    if(1 != vdkSetupEGL(100, 100, VDKS_Val_WindowsWidth, VDKS_Val_WindowsHeight, configAttribs, NULL, ctxAttribs, vdk_egl))
    {
        return VDKS_FALSE;
    }

    /* Adjust the window size to make sure these size values does not go beyound the screen limits. */
    vdkGetWindowInfo((*vdk_egl).window, NULL, NULL, &VDKS_Val_WindowsWidth, &VDKS_Val_WindowsHeight, NULL, NULL);

    return VDKS_TRUE;
}
#endif
/*
 * Data
*/
VDKS_BOOL VDKS_ReadFloats(const char* Path, float **Buffer, SIZE_T *Size)
{
    FILE* fp = NULL;

    float* buf = NULL;

    unsigned long i = 0;

    fp = fopen(Path, "r");
    if (NULL == fp) {
        printf("VDKS_ReadFloats: failed to open file: %s", Path);
        return VDKS_FALSE;
    }

    fscanf(fp, "%lu\n", Size);

    buf = (float *)malloc(sizeof(float) * (*Size));
    if (NULL == buf) {
        printf("VDKS_ReadFloats, out of memory");
        fclose(fp);
        return VDKS_FALSE;
    }

    while (i < (*Size))
    {
        if (1 != fscanf(fp, "%f ", &(buf[i])))
        {
            printf("VDKS_ReadFloats : No enough data.");
            fclose(fp);
            return VDKS_FALSE;
        }

        i++;
    }

    *Buffer = buf;
    fclose(fp);
    return VDKS_TRUE;
}

VDKS_BOOL VDKS_ReadTriangle(const char* Path, unsigned short **Buffer, SIZE_T *VertexCount)
{
    FILE* fp = NULL;

    unsigned short* buf = NULL;

    unsigned long i = 0;

    unsigned int tmp = 0;

    fp = fopen(Path, "r");
    if (NULL == fp) {
        printf("VDKS_ReadTriangle : failed to open file: %s\n", Path);
        return VDKS_FALSE;
    }

    fscanf(fp, "%lu\n", VertexCount);

    *VertexCount = (*VertexCount) * 3;

    buf = (unsigned short*)malloc(sizeof(unsigned short) * (*VertexCount));
    if (NULL == buf) {
        printf("VDKS_ReadTriangle : out of memory\n");
        fclose(fp);
        return VDKS_FALSE;
    }

    while (i < (*VertexCount))
    {
        if (1 != fscanf(fp, "%d ", &tmp))
        {
            printf("VDKS_ReadTriangle : No enough data.");
            fclose(fp);
            return VDKS_FALSE;
        }

        buf[i] = (unsigned short)(tmp & 0x0FFFF);

        i++;
    }

    *Buffer = buf;
    fclose(fp);

    return VDKS_TRUE;
}

float VDKS_Func_Distance(float x1, float y1, float z1, float x2, float y2, float z2)
{
    return (float)sqrt(
        (x1 - x2) * (x1 - x2) +
        (y1 - y2) * (y1 - y2) +
        (z1 - z2) * (z1 - z2)
        );
}

void VDKS_Func_ModelCenterRadius(float* Position, int PositionFloatCount, float* X, float* Y, float* Z, float * R)
{
    int i;

    int vertex_count = PositionFloatCount / 3;

    float acc_x = 0.0f;
    float acc_y = 0.0f;
    float acc_z = 0.0f;

    float max_r = 0.0f;

    for (i = 0; i < vertex_count; i++)
    {
        acc_x += Position[i * 3 + 0];
        acc_y += Position[i * 3 + 1];
        acc_z += Position[i * 3 + 2];
    }

    *X = acc_x / vertex_count;
    *Y = acc_y / vertex_count;
    *Z = acc_y / vertex_count;

    for (i = 0; i < vertex_count; i++)
    {
        float dis =
        VDKS_Func_Distance(
                Position[i * 3 + 0],
                Position[i * 3 + 1],
                Position[i * 3 + 2],
                *X,
                *Y,
                *Z);

        if (max_r < dis)
        {
            max_r = dis;
        }
    }

    *R = max_r;
}

void VDKS_Func_Model_LeftNearBottomRightFarTop(
    float* Position, int PositionFloatCount,
    float* MAX_X, float* MAX_Y, float* MAX_Z,
    float* MIN_X, float* MIN_Y, float* MIN_Z)
{
    int i;

    int vertex_count = PositionFloatCount / 3;

    float max_x = 0.0f;
    float min_x = 0.0f;

    float max_y = 0.0f;
    float min_y = 0.0f;

    float max_z = 0.0f;
    float min_z = 0.0f;

    for (i = 0; i < vertex_count; i++)
    {
        max_x = max(Position[i * 3 + 0], max_x);
        max_y = max(Position[i * 3 + 1], max_y);
        max_z = max(Position[i * 3 + 2], max_z);

        min_x = min(Position[i * 3 + 0], min_x);
        min_y = min(Position[i * 3 + 1], min_y);
        min_z = min(Position[i * 3 + 2], min_z);
    }

    *MAX_X = max_x;
    *MAX_Y = max_y;
    *MAX_Z = max_z;

    *MIN_X = min_x;
    *MIN_Y = min_y;
    *MIN_Z = min_z;
}

void VDKS_Func_CheckGLError(char* file, int line)
{
    GLint error = GL_NO_ERROR;

    error = glGetError();

    if (error != GL_NO_ERROR)
    {
        char* str = "GL_NO_ERROR";

        if(error == GL_INVALID_ENUM)
        {
            str = "GL_INVALID_ENUM";
        }
        else if(error == GL_INVALID_FRAMEBUFFER_OPERATION)
        {
            str = "GL_INVALID_FRAMEBUFFER_OPERATION";
        }
        else if(error == GL_INVALID_VALUE)
        {
            str = "GL_INVALID_VALUE";
        }
        else if(error == GL_INVALID_OPERATION)
        {
            str = "GL_INVALID_OPERATION";
        }
        else if(error == GL_OUT_OF_MEMORY)
        {
            str = "GL_OUT_OF_MEMORY";
        }
        else
        {
            str = "Unknown Error";
        }

        printf("GL Error : \n\t"
            "file : %s, line : %d\n\t"
            "error-string : %s\n",
            file, line,
            str);
    }
}

/*
 * BMP Operations.
*/

#define BF_TYPE 0x4D42             /* "MB" */

typedef struct RGB {                      /**** Colormap entry structure ****/
    unsigned char   rgbBlue;          /* Blue value */
    unsigned char   rgbGreen;         /* Green value */
    unsigned char   rgbRed;           /* Red value */
    unsigned char   rgbReserved;      /* Reserved */
}
RGB;

typedef struct BMPINFOHEADER {                     /**** BMP file info structure ****/
    unsigned int   biSize;           /* Size of info header */
    int            biWidth;          /* Width of image */
    int            biHeight;         /* Height of image */
    unsigned short biPlanes;         /* Number of color planes */
    unsigned short biBitCount;       /* Number of bits per pixel */
    unsigned int   biCompression;    /* Type of compression to use */
    unsigned int   biSizeImage;      /* Size of image data */
    int            biXPelsPerMeter;  /* X pixels per meter */
    int            biYPelsPerMeter;  /* Y pixels per meter */
    unsigned int   biClrUsed;        /* Number of colors used */
    unsigned int   biClrImportant;   /* Number of important colors */
}
BMPINFOHEADER;

typedef struct BMPINFO {                      /**** Bitmap information structure ****/
    BMPINFOHEADER   bmiHeader;      /* Image header */
    RGB             bmiColors[256]; /* Image colormap */
}
BMPINFO;

typedef struct BMPFILEHEADER {                      /**** BMP file header structure ****/
    unsigned short bfType;           /* Magic number for file */
    unsigned int   bfSize;           /* Size of file */
    unsigned short bfReserved1;      /* Reserved */
    unsigned short bfReserved2;      /* ... */
    unsigned int   bfOffBits;        /* Offset to bitmap data */
}
BMPFILEHEADER;

/*
 * 'read_word()' - Read a 16-bit unsigned integer.
 */
static unsigned short read_word(FILE *fp)
{
    unsigned char b0, b1; /* Bytes from file */

    b0 = getc(fp);
    b1 = getc(fp);

    return ((b1 << 8) | b0);
}

/*
 * 'read_dword()' - Read a 32-bit unsigned integer.
 */
static unsigned int read_dword(FILE *fp)
{
    unsigned char b0, b1, b2, b3; /* Bytes from file */

    b0 = getc(fp);
    b1 = getc(fp);
    b2 = getc(fp);
    b3 = getc(fp);

    return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

/*
 * 'read_long()' - Read a 32-bit signed integer.
 */
static int read_long(FILE *fp)
{
    unsigned char b0, b1, b2, b3; /* Bytes from file */

    b0 = getc(fp);
    b1 = getc(fp);
    b2 = getc(fp);
    b3 = getc(fp);

    return ((int)(((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

unsigned char * GltLoadDIBitmap(const char *filename, BMPINFO *info)
{
    FILE            *fp;          /* Open file pointer */
    unsigned char   *bits;        /* Bitmap pixel bits */
    unsigned char   *ptr;         /* Pointer into bitmap */
    unsigned char   temp;         /* Temporary variable to swap red and blue */
    int             x, y;         /* X and Y position in image */
    int             length;       /* Line length */
    unsigned int    bitsize;      /* Size of bitmap */
    int             infosize;     /* Size of header information */
    int                bpp;
    BMPFILEHEADER   header;       /* File header */

    /* Try opening the file; use "rb" mode to read this *binary* file. */
    if ((fp = fopen(filename, "rb")) == NULL)
        return (NULL);

    /* Read the file header and any following bitmap information... */
    header.bfType      = read_word(fp);
    header.bfSize      = read_dword(fp);
    header.bfReserved1 = read_word(fp);
    header.bfReserved2 = read_word(fp);
    header.bfOffBits   = read_dword(fp);

    if (header.bfType != BF_TYPE){
        /* Not a bitmap file - return NULL... */
        fclose(fp);
        return (NULL);
    }

    infosize = header.bfOffBits - 18;

    info->bmiHeader.biSize          = read_dword(fp);
    info->bmiHeader.biWidth         = read_long(fp);
    info->bmiHeader.biHeight        = read_long(fp);
    info->bmiHeader.biPlanes        = read_word(fp);
    info->bmiHeader.biBitCount      = read_word(fp);
    info->bmiHeader.biCompression   = read_dword(fp);
    info->bmiHeader.biSizeImage     = read_dword(fp);
    info->bmiHeader.biXPelsPerMeter = read_long(fp);
    info->bmiHeader.biYPelsPerMeter = read_long(fp);
    info->bmiHeader.biClrUsed       = read_dword(fp);
    info->bmiHeader.biClrImportant  = read_dword(fp);

    if (infosize > 40){
        if (fread(info->bmiColors, infosize - 40, 1, fp) < 1){
            /* Couldn't read the bitmap header - return NULL... */
            fclose(fp);
            return (NULL);
        }
    }

    /* Now that we have all the header info read in, allocate memory for *
     * the bitmap and read *it* in...                                    */
    if ((bitsize = info->bmiHeader.biSizeImage) == 0)
        bitsize = (info->bmiHeader.biWidth *
                   info->bmiHeader.biBitCount + 7) / 8 *
                 abs(info->bmiHeader.biHeight);

    bits = (unsigned char *)malloc( sizeof(unsigned char) * bitsize);

    if ( bits == NULL )
    {
        /* Couldn't allocate memory - return NULL! */
        fclose(fp);
        return (NULL);
    }

    if (fread(bits, 1, bitsize, fp) < bitsize)
    {
        /* Couldn't read bitmap - free memory and return NULL! */
        free(bits);
        fclose(fp);
        return (NULL);
    }

    /* Swap red and blue */
    //length = ((*info)->bmiHeader.biWidth * 3 + 3) & ~3;
    bpp = info->bmiHeader.biBitCount / 8;            //bytes per pixel
    length = info->bmiHeader.biWidth * bpp;
    for (y = 0; y < info->bmiHeader.biHeight; y ++){
        for (ptr = bits + y * length, x = info->bmiHeader.biWidth;
             x > 0; x --, ptr += bpp){
            temp   = ptr[2];
            ptr[2] = ptr[0];
            ptr[0] = temp;
        }
    }

    /* OK, everything went fine - return the allocated bitmap... */
    fclose(fp);
    return (bits);
}

/*
*    The result bytes stream is from left-bottom corner and goes up by left to right.
*/
unsigned char * VDKS_Func_ReadBmp_Bpp(const char* filename, int * width, int * height, int *  bytepp)
{
#ifndef ANDROID
    BMPINFO info = { 0 };
#else
    BMPINFO info ;
#endif
    unsigned char * rt = GltLoadDIBitmap(filename, &info);

    if (rt != NULL)
    {
        if (info.bmiHeader.biBitCount == 24)
        {
            *bytepp = 3;
        }
        else if (info.bmiHeader.biBitCount == 32)
        {
            *bytepp = 4;
        }
        else if (info.bmiHeader.biBitCount == 8)
        {
            *bytepp = 1;
        }
        else
        {
            *bytepp = 0;
        }

        *width =  info.bmiHeader.biWidth;
        *height = info.bmiHeader.biHeight;
    }

    return rt;
}

unsigned char * VDKS_Func_ReadBmp(char* filename, int * width, int * height)
{
#ifndef ANDROID
    BMPINFO info = { 0 };
#else
    BMPINFO info ;
#endif
    unsigned char * rt = GltLoadDIBitmap(filename, &info);

    if (rt != NULL)
    {
        *width =  info.bmiHeader.biWidth;
        *height = info.bmiHeader.biHeight;
    }

    return rt;
}


unsigned char * VDKS_Func_TransformBmp(unsigned char * Pixels, int width, int height, VDKS_BOOL X, VDKS_BOOL Y)
{
    unsigned int size = sizeof(unsigned char) * 4 * width * height;
    unsigned char * rt =
            (unsigned char *)malloc(size);
    if (NULL == rt) {
        printf("VDKS_Func_TransformBmp : out-of-memory\n");
        return NULL;
    }

    if(X == VDKS_TRUE)
    {
        int x, y;
        for(y = 0; y < height; y++)
        {
            for(x = 0; x < width; x++)
            {
                int offset_dst = y * width * 4 + x * 4;
                int offset_src = y * width * 4 + (width - 1 - x) * 4;

                int i;
                for(i = 0; i < 4; i++)
                {
                    rt[offset_dst + i] = Pixels[offset_src + i];
                }
            }
        }
    }
    else
    {
        memcpy(rt, Pixels, size);
    }


    if(Y == VDKS_TRUE)
    {
        int y;
        unsigned char * temp =  (unsigned char *)malloc(size);
        if (NULL == temp) {
            printf("VDKS_Func_TransformBmp: out-of-memory\n");
            free(rt);
            return NULL;
        }
        for(y = 0; y < height; y++)
        {
            int offset_dst = y * width * 4;
            int offset_src = (height - 1 - y) * width * 4;

            memcpy(&(temp[offset_dst]), &(rt[offset_src]), (unsigned int)(size/height));
        }

        free(rt);

        rt = temp;
    }

    return rt;
}

#define BIT_RGB       0             /* No compression - straight BGR data */
#define BIT_RLE8      1             /* 8-bit run-length compression */
#define BIT_RLE4      2             /* 4-bit run-length compression */
#define BIT_BITFIELDS 3             /* RGB bitmap with RGB masks */

/*
 * 'write_word()' - Write a 16-bit unsigned integer.
 */
static int write_word(FILE *fp, unsigned short w)
{
    putc(w, fp);
    return (putc(w >> 8, fp));
}

/*
 * 'write_dword()' - Write a 32-bit unsigned integer.
 */
static int write_dword(FILE *fp, unsigned int dw)
{
    putc(dw, fp);
    putc(dw >> 8, fp);
    putc(dw >> 16, fp);
    return (putc(dw >> 24, fp));
}

/*
 * 'write_long()' - Write a 32-bit signed integer.
 */
static int write_long(FILE *fp,int  l)
{
    putc(l, fp);
    putc(l >> 8, fp);
    putc(l >> 16, fp);
    return (putc(l >> 24, fp));
}

/*
 * 'SaveDIBitmap()' - Save a DIB/BMP file to disk.
 *
 * Returns 0 on success or -1 on failure...
 */

int GltSaveDIBitmap(const char *filename, BMPINFO *info, unsigned char *bits)
{
    FILE *fp;                      /* Open file pointer */
    unsigned int    size,                     /* Size of file */
                    infosize,                 /* Size of bitmap info */
                    bitsize;                  /* Size of bitmap pixels */


    /* Try opening the file; use "wb" mode to write this *binary* file. */
    if ((fp = fopen(filename, "wb")) == NULL)
        return (-1);

    /* Figure out the bitmap size */
    if (info->bmiHeader.biSizeImage == 0)
        bitsize =  (info->bmiHeader.biWidth *
                    info->bmiHeader.biBitCount + 7) / 8 *
                abs(info->bmiHeader.biHeight);
    else
        bitsize = info->bmiHeader.biSizeImage;

    /* Figure out the header size */
    infosize = sizeof(BMPINFOHEADER);
    switch (info->bmiHeader.biCompression)
    {
    case BIT_BITFIELDS :
        infosize += 12; /* Add 3 RGB doubleword masks */
        if (info->bmiHeader.biClrUsed == 0)
        break;
    case BIT_RGB :
        if (info->bmiHeader.biBitCount > 8 &&
        info->bmiHeader.biClrUsed == 0)
        break;
    case BIT_RLE8 :
    case BIT_RLE4 :
        if (info->bmiHeader.biClrUsed == 0)
            infosize += (1 << info->bmiHeader.biBitCount) * 4;
        else
            infosize += info->bmiHeader.biClrUsed * 4;
        break;
    }

    size = sizeof(BMPFILEHEADER) + infosize + bitsize;

    /* Write the file header, bitmap information, and bitmap pixel data... */
    write_word(fp, BF_TYPE);        /* bfType */
    write_dword(fp, size);          /* bfSize */
    write_word(fp, 0);              /* bfReserved1 */
    write_word(fp, 0);              /* bfReserved2 */
    write_dword(fp, 14 + infosize); /* bfOffBits */

    write_dword(fp, info->bmiHeader.biSize);
    write_long(fp, info->bmiHeader.biWidth);
    write_long(fp, info->bmiHeader.biHeight);
    write_word(fp, info->bmiHeader.biPlanes);
    write_word(fp, info->bmiHeader.biBitCount);
    write_dword(fp, info->bmiHeader.biCompression);
    write_dword(fp, info->bmiHeader.biSizeImage);
    write_long(fp, info->bmiHeader.biXPelsPerMeter);
    write_long(fp, info->bmiHeader.biYPelsPerMeter);
    write_dword(fp, info->bmiHeader.biClrUsed);
    write_dword(fp, info->bmiHeader.biClrImportant);

    if (infosize > 40)
    if (fwrite(info->bmiColors, infosize - 40, 1, fp) < 1)
    {
        /* Couldn't write the bitmap header - return... */
        fclose(fp);
        return (-1);
    }

    if (fwrite(bits, 1, bitsize, fp) < bitsize)
    {
        /* Couldn't write the bitmap - return... */
        fclose(fp);
        return (-1);
    }

    /* OK, everything went fine - return... */
    fclose(fp);

    return (0);
}

/*
 *  Name:       SaveImage( const char* file_name, int width, int height, void* pixels )
 *  Returns:    None.
 *  Parameters: file_name, the name of the image file to be saved;
 *              width, height, size of the image;
 *              pixels, the pixel data of the image.
 *  Description:This interface is used to save an image file from the given pixel data in format RGBA8888. Currently we save it in bmp format.
*/
void    SaveImage( const char* fileName, int width, int height, void* pixels )
{
    unsigned char* pBase = (unsigned char*)pixels;
    unsigned char temp;
    int pixels_count = width *height;

    int i;

    BMPINFO  bmpInfo;
    bmpInfo.bmiHeader.biSize = sizeof( bmpInfo.bmiHeader );
    bmpInfo.bmiHeader.biBitCount = 32;        //RGBA8888
    bmpInfo.bmiHeader.biWidth = width;
    bmpInfo.bmiHeader.biHeight = height;
    bmpInfo.bmiHeader.biSizeImage = width * height * 4;
    bmpInfo.bmiHeader.biCompression = BIT_RGB;
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biClrUsed = 0;
    bmpInfo.bmiHeader.biClrImportant = 0;
    bmpInfo.bmiHeader.biXPelsPerMeter = bmpInfo.bmiHeader.biYPelsPerMeter = 0;


    for(i = 0; i < pixels_count; ++i ){        //Swap Blue and Red.
        temp = pBase[0];
        pBase[0] = pBase[2];
        pBase[2] = temp;
        pBase += 4;
    }
    GltSaveDIBitmap( fileName, &bmpInfo, (unsigned char*)pixels );
}

/*
*    The first pixel should be the lower left corner of the picture.
*/
void VDKS_Func_SaveBmp(const char* fileName, int width, int height, void* pixels)
{
    SaveImage( fileName, width, height, pixels );
    return;
}

/*
 * Location Manager
*/

/*
 * NOT THREAD SAFE!!!
*/

#define VDKS_Macro_LocationInUse        1
#define VDKS_Macro_LocationNotInUse        0

static GLuint * _VDKS_LocationResource = NULL;
static GLuint    _VDKS_LocationResourceCount = 0;

GLint VDKS_Func_LocationManagerInit()
{
    GLint max_att = 0;

    GLint i;

    if(_VDKS_LocationResource)
    {
        assert (_VDKS_LocationResourceCount >= 1);
        return _VDKS_LocationResourceCount;
    }

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_att);

    assert(max_att >= 1);

    _VDKS_LocationResourceCount = max_att;

    _VDKS_LocationResource = (GLuint *)malloc(sizeof(GLuint) * max_att);

    assert(_VDKS_LocationResource != NULL);

    for (i = 0; i < max_att; i++)
    {
        _VDKS_LocationResource[i] = VDKS_Macro_LocationNotInUse;
    }

    return max_att;
}

void VDKS_Func_LocationManagerDestroy()
{
    if (!_VDKS_LocationResource)
    {
        return;
    }
    else
    {
        free(_VDKS_LocationResource);

        _VDKS_LocationResource = NULL;
        _VDKS_LocationResourceCount = 0;
    }
}

GLuint VDKS_Func_LocationAcquire()
{
    GLuint rt = 0;

    GLuint i = 0;

    assert(_VDKS_LocationResourceCount >= 1 && _VDKS_LocationResource != NULL);

    for (i = 1; i < _VDKS_LocationResourceCount; i++)
    {
        if (_VDKS_LocationResource[i] == VDKS_Macro_LocationNotInUse)
        {
            rt = i;

            _VDKS_LocationResource[i] = VDKS_Macro_LocationInUse;

            break;
        }
    }

    return rt;
}

void VDKS_Func_LocationRelease(GLuint Location)
{
    if (_VDKS_LocationResource == NULL)
    {
        return;
    }

    if (Location >= _VDKS_LocationResourceCount)
    {
        return;
    }

    _VDKS_LocationResource[Location] = VDKS_Macro_LocationNotInUse;
}

/*
*
*/
void VDKS_Func_DisableAllVertexAttributeArray()
{
    GLint count = 0;

    GLint i;

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &count);

    for (i = 0; i < count; i++)
    {
        glDisableVertexAttribArray(i);
    }
}


/*
*    Vertex Attribute Array Manager
*/

void VDKS_Func_Program_PresetAttributesLocations(
    GLuint Program,
    VDKS_Struct_AttributeInformation* IndexBindings,
    int IndexBindingCount
    )
{
    int i;
    assert(Program);
    /*
    *    Pre-binding the general indices.
    */
    for(i = 0; i < IndexBindingCount; i++)
    {
        glBindAttribLocation(Program, *(IndexBindings[i].general_index), IndexBindings[i].name);
    }
}

VDKS_BOOL VDKS_Func_Program_QueryActiveAttributesCheckConsistent(
    GLuint Program,
    VDKS_Struct_AttributeInformation* IndexBindings,
    int IndexBindingCount)
{

    int        index;
    GLint    nr_act_att = 0;
    char *    name  = NULL;
    GLsizei bufsize = 128;

    for (index = 0; index < IndexBindingCount; index ++)
    {
        IndexBindings[index].in_use = 0;
    }

    /*
    *    Check the program.
    */
    assert(Program);

    glGetProgramiv(Program, GL_ACTIVE_ATTRIBUTES, &nr_act_att);

    /*
    *    Name buffer to save the query result.
    */
    name = (char *)malloc(bufsize);
    if (NULL == name) {
        printf("QueryActiveAttributesCheckConsistent : out-of-memory\n");
        return VDKS_FALSE;
    }

    for (index = 0; index < nr_act_att; index++)
    {
        int binding;
        GLuint general_index = 0;
        GLsizei length;
        GLenum    type;
        GLint    size = 0;

        glGetActiveAttrib(Program, index, bufsize, &length, &size, &type, name);

        general_index = glGetAttribLocation(Program, name);

        for(binding = 0; binding < IndexBindingCount; binding++)
        {
            if(strcmp(IndexBindings[binding].name, name) != 0)
            {
                continue;
            }

            if (general_index != *(IndexBindings[binding].general_index))
            {
                printf(
                    "Attribute Name : %s, "
                    "general location in program: %d, "
                    "general location declared in interface : %d.\n",
                    IndexBindings[binding].name,
                    general_index,
                    *(IndexBindings[binding].general_index));
                free(name);
                return VDKS_FALSE;
            }

            if (size != IndexBindings[binding].glessl_size)
            {
                printf(
                    "Attribute Name : %s, "
                    "size in program: %d, "
                    "size declared in interface : %d.\n",
                    IndexBindings[binding].name,
                    size,
                    IndexBindings[binding].glessl_size);

                free(name);
                return VDKS_FALSE;
            }


            if (type != IndexBindings[binding].glessl_type)
            {
                printf(
                    "Attribute Name : %s, "
                    "query_type in program: %d, "
                    "query_type declared in interface : %d.\n",
                    IndexBindings[binding].name,
                    type,
                    IndexBindings[binding].glessl_type);

                free(name);
                return VDKS_FALSE;
            }

            IndexBindings[index].in_use = 1;

            break;
        }/*use binding to walk through.*/

        if (binding == IndexBindingCount)
        {
            printf("Attribute Name : %s can not be found in the interface.", name);
            free(name);
            return VDKS_FALSE;
        }
    }

    free(name);

    return VDKS_TRUE;
}


VDKS_BOOL VDKS_Func_BufferObject_SetUsage(
    VDKS_Struct_AttributeInformation* IndexBindings,
    int IndexBindingCount)
{
    int i;

    for (i = 0; i < IndexBindingCount; i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, *(IndexBindings[i].gl_bufobj_name));

        glVertexAttribPointer(
            *(IndexBindings[i].general_index),
            IndexBindings[i].size_of_type,
            IndexBindings[i].type,
            IndexBindings[i].normalized,
            IndexBindings[i].stride,
            IndexBindings[i].ptr);
    }

    return VDKS_TRUE;
}

VDKS_BOOL VDKS_Func_ActiveAttribute_LocationEnable(
    VDKS_Struct_AttributeInformation* IndexBindings,
    int IndexBindingCount)
{
    int i;

    for (i = 0; i < IndexBindingCount; i++)
    {
        if (IndexBindings[i].in_use == 1)
        {
            glEnableVertexAttribArray(*(IndexBindings[i].general_index));
        }
    }

    return VDKS_TRUE;
}

/*
*    Uniform Manager
*/

VDKS_BOOL VDKS_Func_Program_SettingUniforms(
    GLuint Program,
    VDKS_Struct_UnifomInfomation * Uniforms,
    int UniformCount
    )
{
    int i;

    glUseProgram(Program);

    for (i = 0; i < UniformCount; i++)
    {
        if (Uniforms[i].in_use == 0)
        {
            continue;
        }

        if (*(Uniforms[i].program) != Program)
        {
            continue;
        }

        switch(Uniforms[i].glessl_type)
        {
        default:

            printf("VDKS_Func_Program_SettingUniforms : Uniform : %s, type unknown.\n", Uniforms[i].name);
            return VDKS_FALSE;

        case GL_FLOAT:
            glUniform1fv(Uniforms[i].location, Uniforms[i].glessl_size, (const GLfloat *)Uniforms[i].ptr);
            break;

        case GL_FLOAT_VEC2 :
            glUniform2fv(Uniforms[i].location, Uniforms[i].glessl_size, (const GLfloat *)Uniforms[i].ptr);
            break;

        case GL_FLOAT_VEC3 :
            glUniform3fv(Uniforms[i].location, Uniforms[i].glessl_size, (const GLfloat *)Uniforms[i].ptr);
            break;

        case GL_FLOAT_VEC4 :
            glUniform4fv(Uniforms[i].location, Uniforms[i].glessl_size, (const GLfloat *)Uniforms[i].ptr);
            break;

        case GL_INT_VEC2 :
            glUniform2iv(Uniforms[i].location, Uniforms[i].glessl_size, (const GLint *)Uniforms[i].ptr);
            break;

        case GL_INT_VEC3 :
            glUniform3iv(Uniforms[i].location, Uniforms[i].glessl_size, (const GLint *)Uniforms[i].ptr);
            break;

        case GL_INT_VEC4 :
            glUniform4iv(Uniforms[i].location, Uniforms[i].glessl_size, (const GLint *)Uniforms[i].ptr);
            break;

        case GL_BOOL :
            glUniform1iv(Uniforms[i].location, Uniforms[i].glessl_size, (const GLint *)Uniforms[i].ptr);
            break;

        case GL_BOOL_VEC2 :
            glUniform2fv(Uniforms[i].location, Uniforms[i].glessl_size, (const GLfloat *)Uniforms[i].ptr);
            break;

        case GL_BOOL_VEC3 :
            glUniform3fv(Uniforms[i].location, Uniforms[i].glessl_size, (const GLfloat *)Uniforms[i].ptr);
            break;

        case GL_BOOL_VEC4 :
            glUniform4fv(Uniforms[i].location, Uniforms[i].glessl_size, (const GLfloat *)Uniforms[i].ptr);
            break;

        case GL_FLOAT_MAT2 :
            glUniformMatrix2fv(Uniforms[i].location, Uniforms[i].glessl_size, Uniforms[i].transpose, (const GLfloat *)Uniforms[i].ptr);
            break;

        case GL_FLOAT_MAT3 :
            glUniformMatrix3fv(Uniforms[i].location, Uniforms[i].glessl_size, Uniforms[i].transpose, (const GLfloat *)Uniforms[i].ptr);
            break;

        case GL_FLOAT_MAT4 :
            glUniformMatrix4fv(Uniforms[i].location, Uniforms[i].glessl_size, Uniforms[i].transpose, (const GLfloat *)Uniforms[i].ptr);
            break;

        case GL_SAMPLER_2D :
            glUniform1iv(Uniforms[i].location, Uniforms[i].glessl_size, (const GLint *)Uniforms[i].ptr);
            break;

        case GL_SAMPLER_CUBE :
            glUniform1iv(Uniforms[i].location, Uniforms[i].glessl_size, (const GLint *)Uniforms[i].ptr);
            break;
        }

    }/*for*/

    glUseProgram(0);

    return VDKS_TRUE;
}

VDKS_BOOL VDKS_Func_Program_ValidateUniformsGetLocations(
    GLuint Program,
    VDKS_Struct_UnifomInfomation * Uniforms,
    int UniformCount
    )
{
    GLint nr_act_uni = 0;

    GLint i;

    char *    name  = NULL;
    GLsizei bufsize = 128;

    assert(Program);

    /*
    *    Set in_use.
    */
    for(i = 0; i < UniformCount; i++)
    {
        Uniforms[i].in_use = 0;
    }

    name = (char *)malloc(bufsize);
    if (NULL == name) {
        printf("ValidateUniformsGetLocations : out-of-memory\n");
        return VDKS_FALSE;
    }

    glGetProgramiv(Program, GL_ACTIVE_UNIFORMS, &nr_act_uni);

    glUseProgram(Program);

    for(i = 0; i < nr_act_uni; i++)
    {
        int uniform_walker;
        GLsizei length;
        GLenum    type;
        GLint    size = 0;

        glGetActiveUniform(Program, i, bufsize, &length, &size, &type, name);

        for (uniform_walker = 0; uniform_walker < UniformCount; uniform_walker++)
        {
            if (*(Uniforms[uniform_walker].program) != Program)
            {
                continue;
            }

            if (!strcmp(name, Uniforms[uniform_walker].name))
            {
                GLuint location = glGetUniformLocation(Program, name);

                if (Uniforms[uniform_walker].glessl_type != type)
                {
                    printf(
                        "Attribute Name : %s, "
                        "type in program: %d, "
                        "query_type declared in interface : %d.\n",
                        Uniforms[uniform_walker].name,
                        type,
                        Uniforms[uniform_walker].glessl_type);

                    free(name);
                    return VDKS_FALSE;
                }

                if (Uniforms[uniform_walker].glessl_size != size)
                {
                    printf(
                        "Attribute Name : %s, "
                        "size in program: %d, "
                        "query_size declared in interface : %d.\n",
                        Uniforms[uniform_walker].name,
                        size,
                        Uniforms[uniform_walker].glessl_size);

                    free(name);
                    return VDKS_FALSE;
                }

                Uniforms[uniform_walker].in_use = 1;

                Uniforms[uniform_walker].location = location;

                break;
            }/*find the name*/
        }/*walk through all input uniforms*/

        if (uniform_walker == UniformCount)
        {
            printf("Uniform name %s,can not be found.", name);
            free(name);
            return VDKS_FALSE;
        }

    }/*walk through all active uniforms.*/

    free(name);

    return VDKS_TRUE;
}

/*
*    Fur Effect Utility Routines
*/

unsigned char * VDKS_Func_MakeRandAlphaRect(unsigned RandSeed, unsigned Size, unsigned WhiteDensity, unsigned WhiteRadius)
{
    unsigned char * rect = NULL;

    unsigned i;

    assert (Size >= 1 && WhiteDensity >= 1 && Size <= RAND_MAX);

    rect = (unsigned char *)malloc(Size * Size);
    if (NULL == rect) {
        printf("VDKS_Func_MakeRandAlphaRect : out-of-memory\n");
        return rect;
    }

    for(i = 0; i < Size * Size; i++)
    {
        rect[i] = 0;
    }

    srand(RandSeed);

    for(i  = 0; i < WhiteDensity; i++)
    {
        unsigned w =0;
        unsigned h = 0;

        unsigned j = 0;
        unsigned k = 0;

        w = rand() % Size;
        h = rand() % Size;

        rect[h * Size + w] = 255;

        for(j = 0; j < WhiteRadius; j++)
        {
            for(k = 0; k < WhiteRadius; k++)
            {
                unsigned ww = w;
                unsigned hh = h;

                ww = (w - j) % Size;
                hh = (h - k) % Size;
                rect[hh * Size + ww] = 255;

                ww = (w + j) % Size;
                hh = (h + k) % Size;
                rect[hh * Size + ww] = 255;
            }
        }

    }

    return rect;
}

GLuint VDKS_Func_MakeRandAlpha2DTexture(unsigned RandSeed, unsigned Size, unsigned WhiteDensity, unsigned WhiteRadius)
{
    GLuint rt = 0;

    GLuint cur_texobj = 0;

    unsigned char * rand_alpha_rect = VDKS_Func_MakeRandAlphaRect(RandSeed, Size, WhiteDensity, WhiteRadius);
    if (NULL == rand_alpha_rect) return rt;

    glGenTextures(1, &rt);

    assert (rt);

    glActiveTexture(GL_TEXTURE0);

    glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint *)&cur_texobj);

    glBindTexture(GL_TEXTURE_2D, rt);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, Size, Size, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, rand_alpha_rect);

    free(rand_alpha_rect);

    glBindTexture(GL_TEXTURE_2D, cur_texobj);

    return rt;
}

GLuint    VDKS_Func_Make2DTexture(const char * BMPFile, int * Width, int * Height)
{
    GLuint rt = 0;

    GLint cur_texobj = 0;

    int width = 0;
    int height = 0;

    int bpp = 0;

    unsigned char * rgba = NULL;

    glGenTextures(1, &rt);

    if (!rt)
    {
        return rt;
    }

    glActiveTexture(GL_TEXTURE0);

    glGetIntegerv(GL_TEXTURE_BINDING_2D, &cur_texobj);

    glBindTexture(GL_TEXTURE_2D, rt);

    rgba = VDKS_Func_ReadBmp_Bpp(BMPFile, &width, &height, &bpp);
    if (NULL == rgba) return 0;

    if (bpp == 3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgba);
    }
    else if(bpp == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    }
    else if(bpp == 1)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, rgba);
    }
    else
    {
        assert(0);
    }

    free(rgba);

    glBindTexture(GL_TEXTURE_2D, cur_texobj);

    if(Width)
    {
        *Width = width;
    }

    if(Height)
    {
        *Height = height;
    }

    return rt;
}
