//  Created by William Talmadge on 6/18/14.
/*
 Copyright (C) 2014  William B. Talmadge
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "FFTSubBlock.h"
#include "ISRe16Rgba.h"

using namespace std;

vector<GLuint> factorInteger(GLuint n)
{
    std::vector<GLuint> result;
    
    while (n%2 == 0) {
        result.push_back(2);
        n = n/2;
    }
    
    for (GLuint i = 3; i*i <= n; i = i+2) {
        while (n%i == 0) {
            result.push_back(i);
            n = n/i;
        }
    }
    
    if (n > 2) {
        result.push_back(n);
    }
    
    return result;
}
vector<GLuint> collectTwos(vector<GLuint> factors)
{
    //Assumes input is sorted
    vector<GLuint> result;
    int i;
    for (i = 0; i < factors.size(); i += 2) {
        if (factors[i] == 2 & factors[i+1] == 2) {
            result.push_back(4);
        } else {
            break;
        }
    }
    result.insert(result.end(), factors.begin() + i, factors.end());
    return result;
}

#pragma mark - Sub block attribute methods
vector<FFTSubBlock::SubBlock>
FFTSubBlock::subBlocks()
{
    vector<SubBlock> result;
    GLuint blockLength = _butterflySize1*_subBlockLength;
    //map over each block, in each block there is one sub block with the given subBlockDigitOut
    for (GLuint blockOffset = 0; blockOffset < _sigSize; blockOffset += blockLength) {

        GLuint subBlockPositionOut = blockOffset + _subBlockDigitOut*_subBlockLength;
        SubBlock subBlock(_butterflySize1, subBlockPositionOut, _subBlockLength);
        //map over the source sub blocks to be summed in the butterfly to get the output sample
        for (GLuint subBlockDigitIn = 0; subBlockDigitIn < _butterflySize1; subBlockDigitIn++) {
            GLuint subBlockPositionIn = blockOffset + subBlockDigitIn*_subBlockLength;
            subBlock.sources[subBlockDigitIn] = TexelSamplerRange{
                    static_cast<GLfloat>(subBlockPositionIn)/_sigSize,
                    static_cast<GLfloat>(subBlockPositionIn + _subBlockLength)/_sigSize
            };
        }
        result.push_back(subBlock);
    }
    return result;
}

vector<FFTSubBlock::TexelSamplerRange>
FFTSubBlock::phaseCorrectSubBlocks()
{
    //TODO: check this thoroughly
    //Note that this is calculating values for the next butterfly stage so that the
    //phase correct indexer can be sub indexed by this drawable's sub blocks, then
    //the phase correct can be applied as a post processing step after a butterfly
    //The pre processing step might be less confusing, but it requires a lot of redundant multiplies.
    assert(_butterflySize2 != 0);
    vector<TexelSamplerRange> result;
    GLuint blockLength = _subBlockLength*_butterflySize1*_butterflySize2; //T_{b+1}
    for (GLuint blockOffset = 0; blockOffset < _sigSize; blockOffset += blockLength) {
        for (GLuint subBlockDigitIn = 0; subBlockDigitIn < _butterflySize2; subBlockDigitIn++) { //s_{M-b-1} ranges R_{M-b-1}
            //GLuint subBlockOffsetIn = subBlockDigitIn*subBlockLength; //s_{M-b-1}T_b
            //GLuint subBlockPositionIn = blockOffset + subBlockOffsetIn;
            
            GLuint phaseTableStride = subBlockDigitIn*_sigSize/blockLength;
            GLuint subSubBlockOffsetIn = _subBlockLength*_subBlockDigitOut; // T_{b-1} k_{M-b}
            //GLuint subSubBlockPositionIn = subBlockPositionIn + subSubBlockOffsetIn;
            
            result.push_back(windowIndexer(phaseTableStride,
                                           subSubBlockOffsetIn,
                                           subSubBlockOffsetIn + _subBlockLength));
       }
    }
    return result;
}
string FFTSubBlock::shaderKey() const
{
    //TODO: leave out the sign if this is a 2 point butterfly, since the direction only changes the phase table in a 2 point butterfly.
    ostringstream s;
    s << "SubBlockButterfly" << _butterflySize1 << ":" << _subBlockDigitOut << ":" << _isPhaseCorrecting << ":" << _sign;
    return s.str();
}
#pragma mark - Butterfly expressions for fragment shader
/*
 Determine what a butterfly should look like using
 butterfly[r_] :=
 Table[Sum[Exp[2 \[Pi] I k s/r] (re[s] + I im[s]), {s, 0, r - 1}], {k,
 0, r - 1}]
 Re[butterfly[3]] // ComplexExpand // FullSimplify
 Im[butterfly[3]] // ComplexExpand // FullSimplify
 */
//FIXME: need a failsafe for bad block digits and other butterfly sizes than 2,3,4,5
void butterfly2Re(ostringstream& s, GLuint subBlockDigitOut)
{
    if (subBlockDigitOut == 0) {
        s << "re0.rgb + re1.rgb;" << endl;
    } else if (subBlockDigitOut == 1) {
        s << "re0.rgb - re1.rgb;" << endl;
    }
    else {
        assert(false); //bad sub block digit provided to drawable for the butterfly size
    }
}
void butterfly2Im(ostringstream& s, GLuint subBlockDigitOut)
{
    if (subBlockDigitOut == 0) {
        s << "im0.rgb + im1.rgb;" << endl;
    } else if (subBlockDigitOut == 1) {
        s << "im0.rgb - im1.rgb;" << endl;
    }
    else {
        assert(false); //bad sub block digit provided to drawable for the butterfly size
    }
}
void butterfly3Re(ostringstream& s, GLuint subBlockDigitOut, int sign)
{
    if (subBlockDigitOut == 0) {
        s << "re0.rgb + re1.rgb + re2.rgb;" << endl;
    } else if (subBlockDigitOut == 1) {
        if (sign > 0) {
            s << "0.5*(-1.732050808*im1.rgb + 1.732050808*im2.rgb + 2.0*re0.rgb - re1.rgb - re2.rgb);" << endl;
        } else {
            s << "0.5*(1.732050808*im1.rgb - 1.732050808*im2.rgb + 2.0*re0.rgb - re1.rgb - re2.rgb);" << endl;
        }
        
    } else if (subBlockDigitOut == 2) {
        if (sign > 0) {
            s << "0.5*(1.732050808*im1.rgb - 1.732050808*im2.rgb + 2.0*re0.rgb - re1.rgb - re2.rgb);" << endl;
        } else {
            s << "0.5*(-1.732050808*im1.rgb + 1.732050808*im2.rgb + 2.0*re0.rgb - re1.rgb - re2.rgb);" << endl;
        }
        
    }
    else {
        assert(false); //bad sub block digit provided to drawable for the butterfly size
    }
}
void butterfly3Im(ostringstream& s, GLuint subBlockDigitOut, int sign)
{
    if (subBlockDigitOut == 0) {
        s << "im0.rgb + im1.rgb + im2.rgb;" << endl;
    } else if (subBlockDigitOut == 1) {
        if (sign > 0) {
            s << "0.5*(2.0*im0.rgb - im1.rgb - im2.rgb + 1.732050808*(re1.rgb - re2.rgb));" << endl;
        } else {
            s << "0.5*(2.0*im0.rgb - im1.rgb - im2.rgb + 1.732050808*(re2.rgb -re1.rgb));" << endl;
        }
    } else if (subBlockDigitOut == 2) {
        if (sign > 0) {
            s << "0.5*(2.0*im0.rgb - im1.rgb - im2.rgb + 1.732050808*(re2.rgb - re1.rgb));" << endl;
        } else {
            s << "0.5*(2.0*im0.rgb - im1.rgb - im2.rgb + 1.732050808*(re1.rgb - re2.rgb));" << endl;
        }
        
    }
    else {
        assert(false); //bad sub block digit provided to drawable for the butterfly size
    }
}
void butterfly4Re(ostringstream& s, GLuint subBlockDigitOut, int sign)
{
    if (subBlockDigitOut == 0) {
        s << "re0.rgb + re1.rgb + re2.rgb + re3.rgb;" << endl;
    } else if (subBlockDigitOut == 1) {
        if (sign > 0) {
            s << "im3.rgb - im1.rgb + re0.rgb - re2.rgb;" << endl;
        } else {
            s << "im1.rgb - im3.rgb + re0.rgb - re2.rgb;" << endl;
        }
    } else if (subBlockDigitOut == 2) {
        if (sign > 0) {
            s << "re0.rgb - re1.rgb + re2.rgb - re3.rgb;" << endl;
        } else {
            s << "re0.rgb - re1.rgb + re2.rgb - re3.rgb;" << endl;
        }
    } else if (subBlockDigitOut == 3) {
        if (sign > 0) {
            s << "im1.rgb - im3.rgb + re0.rgb - re2.rgb;" << endl;
        } else {
            s << "im3.rgb - im1.rgb + re0.rgb - re2.rgb;" << endl;
        }
    }
    else {
        assert(false); //bad sub block digit provided to drawable for the butterfly size
    }
}
void butterfly4Im(ostringstream& s, GLuint subBlockDigitOut, int sign)
{
    if (subBlockDigitOut == 0) {
        s << "im0.rgb + im1.rgb + im2.rgb + im3.rgb;" << endl;
    } else if (subBlockDigitOut == 1) {
        if (sign > 0) {
            s << "im0.rgb - im2.rgb + re1.rgb - re3.rgb;" << endl;
        } else {
            s << "im0.rgb - im2.rgb - re1.rgb + re3.rgb;" << endl;
        }
    } else if (subBlockDigitOut == 2) {
        if (sign > 0) {
            s << "im0.rgb - im1.rgb + im2.rgb - im3.rgb;" << endl;
        } else {
            s << "im0.rgb - im1.rgb + im2.rgb - im3.rgb;" << endl;
        }
    } else if (subBlockDigitOut == 3) {
        if (sign > 0) {
            s << "im0.rgb - im2.rgb - re1.rgb + re3.rgb;" << endl;
        } else {
            s << "im0.rgb - im2.rgb + re1.rgb - re3.rgb;" << endl;
        }
    }
    else {
        assert(false); //bad sub block digit provided to drawable for the butterfly size
    }
}
void butterfly5Re(ostringstream& s, GLuint subBlockDigitOut, int sign)
{
    if (subBlockDigitOut == 0) {
        s << "re0.rgb + re1.rgb + re2.rgb + re3.rgb + re4.rgb;" << endl;
    } else if (subBlockDigitOut == 1) {
        if (sign > 0) {
            s << "re0.rgb + 0.25*(2.35114*(im3.rgb-im2.rgb) + 3.80423*(im4.rgb-im1.rgb) + 1.23607*re1.rgb - 3.23607*re2.rgb - 3.23607*re3.rgb + 1.23607*re4.rgb);" << endl;
        } else {
            s << "re0.rgb + 0.25*(2.35114*(im2.rgb - im3.rgb) + 3.80423*(im1.rgb - im4.rgb) + 1.23607*re1.rgb - 3.23607*re2.rgb - 3.23607*re3.rgb + 1.23607*re4.rgb);" << endl;
        }
    } else if (subBlockDigitOut == 2) {
        if (sign > 0) {
            s << "re0.rgb + 0.25*(3.80423*(im2.rgb - im3.rgb) + 2.35114*(im4.rgb-im1.rgb) - 3.23607*re1.rgb + 1.23607*re2.rgb + 1.23607*re3.rgb - 3.23607*re4.rgb);" << endl;
        } else {
            s << "re0.rgb + 0.25*(3.80423*(im3.rgb - im2.rgb) + 2.35114*(im1.rgb - im4.rgb)  - 3.23607*re1.rgb + 1.23607*re2.rgb + 1.23607*re3.rgb - 3.23607*re4.rgb);" << endl;
        }
        
    } else if (subBlockDigitOut == 3) {
        if (sign > 0) {
            s << "re0.rgb + 0.25*(3.80423*(im3.rgb-im2.rgb) + 2.35114*(im1.rgb - im4.rgb) - 3.23607*re1.rgb + 1.23607*re2.rgb + 1.23607*re3.rgb - 3.23607*re4.rgb);" << endl;
        } else {
            s << "re0.rgb + 0.25*(3.80423*(im2.rgb - im3.rgb) + 2.35114*(im4.rgb - im1.rgb) - 3.23607*re1.rgb + 1.23607*re2.rgb + 1.23607*re3.rgb - 3.23607*re4.rgb);" << endl;
        }
        
    } else if (subBlockDigitOut == 4) {
        if (sign > 0) {
            s << "re0.rgb + 0.25*(2.35114*(im2.rgb - im3.rgb) + 3.80423*(im1.rgb - im4.rgb) + 1.23607*re1.rgb - 3.23607*re2.rgb - 3.23607*re3.rgb + 1.23607*re4.rgb);" << endl;
        } else {
            s << "re0.rgb + 0.25*(2.35114*(im3.rgb - im2.rgb) + 3.80423*(im4.rgb - im1.rgb) + 1.23607*re1.rgb - 3.23607*re2.rgb - 3.23607*re3.rgb + 1.23607*re4.rgb);" << endl;
        }
        
    }
    else {
        assert(false); //bad sub block digit provided to drawable for the butterfly size
    }
}
void butterfly5Im(ostringstream& s, GLuint subBlockDigitOut, int sign)
{
    if (subBlockDigitOut == 0) {
        s << "im0.rgb + im1.rgb + im2.rgb + im3.rgb + im4.rgb;" << endl;
    } else if (subBlockDigitOut == 1) {
        if (sign > 0) {
            s << "im0.rgb + 0.25*(1.23607*im1.rgb - 3.23607*im2.rgb - 3.23607*im3.rgb + 1.23607*im4.rgb + 1.41421*(1.66251*(re2.rgb-re3.rgb) + 2.68999*(re1.rgb-re4.rgb)));" << endl;
        } else {
            s << "im0.rgb + 0.25*(1.23607*im1.rgb - 3.23607*im2.rgb - 3.23607*im3.rgb + 1.23607*im4.rgb + 1.41421*(1.66251*(re3.rgb - re2.rgb) + 2.68999*(re4.rgb - re1.rgb)));" << endl;
        }
        
    } else if (subBlockDigitOut == 2) {
        if (sign > 0) {
            s << "im0.rgb + 0.25*(- 3.23607*im1.rgb + 1.23607*im2.rgb + 1.23607*im3.rgb - 3.23607*im4.rgb + 1.41421*(2.68999*(re3.rgb-re2.rgb) + 1.66251*(re1.rgb-re4.rgb)));" << endl;
        } else {
            s << "im0.rgb + 0.25*(-3.23607*im1.rgb + 1.23607*im2.rgb + 1.23607*im3.rgb - 3.23607*im4.rgb + 1.41421*(2.68999*(re2.rgb - re3.rgb) + 1.66251*(re4.rgb - re1.rgb)));" << endl;
        }
        
    } else if (subBlockDigitOut == 3) {
        if (sign > 0) {
            s << "im0.rgb + 0.25*(1.23607*im2.rgb - 3.23607*im1.rgb + 1.23607*im3.rgb - 3.23607*im4.rgb + 1.41421*(2.68999*(re2.rgb-re3.rgb) + 1.66251*(re4.rgb-re1.rgb)));" << endl;
        } else {
            s << "im0.rgb + 0.25*(-3.23607*im1.rgb + 1.23607*im2.rgb + 1.23607*im3.rgb - 3.23607*im4.rgb + 1.41421*(2.68999*(re3.rgb - re2.rgb) + 1.66251*(re1.rgb - re4.rgb)));" << endl;
        }
        
    } else if (subBlockDigitOut == 4) {
        if (sign > 0) {
            s << "im0.rgb + 0.25*(1.23607*im1.rgb - 3.23607*im2.rgb - 3.23607*im3.rgb + 1.23607*im4.rgb + 1.41421*(1.66251*(re3.rgb-re2.rgb) + 2.68999*(re4.rgb-re1.rgb)));" << endl;
        } else {
            s << "im0.rgb + 0.25*(1.23607*im1.rgb - 3.23607*im2.rgb - 3.23607*im3.rgb + 1.23607*im4.rgb + 1.41421*(1.66251*(re2.rgb - re3.rgb) + 2.68999*(re1.rgb - re4.rgb)));" << endl;
        }
        
    }
    else {
        assert(false); //bad sub block digit provided to drawable for the butterfly size
    }
}
void butterflyRe(ostringstream& s, GLuint butterflySize, GLuint subBlockDigitOut, int sign)
{
    switch (butterflySize) {
        case 2:
            butterfly2Re(s, subBlockDigitOut);
            break;
        case 3:
            butterfly3Re(s, subBlockDigitOut, sign);
            break;
        case 4:
            butterfly4Re(s, subBlockDigitOut, sign);
            break;
        case 5:
            butterfly5Re(s, subBlockDigitOut, sign);
            break;
        default:
            assert(false); //There is no butterfly routine written for the given size
            break;
    }
}
void butterflyIm(ostringstream& s, GLuint butterflySize, GLuint subBlockDigitOut, int sign)
{
    switch (butterflySize) {
        case 2:
            butterfly2Im(s, subBlockDigitOut);
            break;
        case 3:
            butterfly3Im(s, subBlockDigitOut, sign);
            break;
        case 4:
            butterfly4Im(s, subBlockDigitOut, sign);
            break;
        case 5:
            butterfly5Im(s, subBlockDigitOut, sign);
            break;
        default:
            assert(false); //There is no butterfly routine written for the given size
            break;
    }
}
#pragma mark - Shader building blocks
inline void repeatExp(void (*f)(ostringstream&, int), ostringstream& s, int n)
{
    for (int i = 0; i < n; i++) {
        f(s, i);
        s << endl;
    }
}
inline void setInputVaryingExp(ostringstream& s, int n) {
    s << "i" << n << " = " << "iIn" << n << ";";
}
inline void inputAttributeExp(ostringstream& s, int n) {
    s << "attribute highp vec2 iIn" << n << ";";
}
inline void inputVaryingExp(ostringstream& s, int n) {
    s << "varying highp vec2 i" << n << ";";
}
inline void inputTexLookupExp(ostringstream& s, int n) {
    s <<"highp vec4 re" << n << " = texture2D(inputRe, i" << n << ");" << endl;
    s <<"highp vec4 im" << n << " = texture2D(inputIm, i" << n << ");";
}
#pragma mark - Shader source generators
string FFTSubBlock::vertShaderSource()
{
    ostringstream s;
    s << "attribute highp vec4 positionIn;" << endl;
    repeatExp(inputAttributeExp, s, _butterflySize1);
    if (_isPhaseCorrecting) {
        s << "attribute highp vec2 phaseCorrectIn;" << endl;
        s << "varying highp vec2 phaseCorrect;" << endl;
    }
    repeatExp(inputVaryingExp, s, _butterflySize1);
    s << "void main() {" << endl;
    s << "gl_Position = orthoMatrix*positionIn;" << endl;
    if (_isPhaseCorrecting) {
        s << "phaseCorrect = phaseCorrectIn;" << endl;
    }
    repeatExp(setInputVaryingExp, s, _butterflySize1);
    s << "}" << endl;
    return s.str();
}

string FFTSubBlock::fragShaderSource()
{
    ostringstream s;
    repeatExp(inputVaryingExp, s, _butterflySize1);
    if (_isPhaseCorrecting) {
        s << "varying highp vec2 phaseCorrect;" << endl;
        s << "uniform highp sampler2D phaseCorrectTable;" << endl;
    }
    s << "uniform highp sampler2D inputRe;" << endl;
    s << "uniform highp sampler2D inputIm;" << endl;
    s << "const highp float k = 1.0/sqrt(" << _butterflySize1 << ".0);" << endl;
    s << "void main()" << endl;
    s << "{" << endl;
    repeatExp(inputTexLookupExp, s, _butterflySize1);
    s << "highp vec4 result;" << endl;
    s << "result.a = 1.0;" << endl;
    if (_isPhaseCorrecting) {
        s << "highp vec4 p = texture2D(phaseCorrectTable, phaseCorrect);" << endl;
        s << "highp vec4 resultRe;" << endl;
        s << "highp vec4 resultIm;" << endl;
        s << "resultRe.a = 1.0;" << endl;
        s << "resultIm.a = 1.0;" << endl;
        s << "resultRe.rgb = ";
        butterflyRe(s, _butterflySize1, _subBlockDigitOut, _sign);
        s << "resultIm.rgb = ";
        butterflyIm(s, _butterflySize1, _subBlockDigitOut, _sign);
        if (_outType == OutType::Real) {
            s << "result.rgb = p.r*resultRe.rgb - p.g*resultIm.rgb;" << endl;
        } else if (_outType == OutType::Imag) {
            s << "result.rgb = p.r*resultIm.rgb + p.g*resultRe.rgb;" << endl;
        }
    } else {
        s << "result.rgb = ";
        if (_outType == OutType::Real) {
            butterflyRe(s, _butterflySize1, _subBlockDigitOut, _sign);
        } else if (_outType == OutType::Imag) {
            butterflyIm(s, _butterflySize1, _subBlockDigitOut, _sign);
        }
    }
    s << "gl_FragColor = k*result;" << endl;
    s << "}" << endl;
    DLPRINT("%s\n", s.str().c_str());
    return s.str();
}
#pragma mark - Phase table creation and managment
//TODO: switch to phase table class
unordered_map<int, ISTextureRef> FFTSubBlock::_phaseTableCache = unordered_map<int, ISTextureRef>();
static string phaseTableVertSource = SHADER_STRING
(
 attribute vec4 positionIn;
 attribute highp float kIn;

 uniform mat4 orthoMatrix;
 
 varying highp float k;
 void main()
 {
     gl_Position = orthoMatrix*positionIn;
     k = kIn;
 }
);
static string phaseTableFragSource = SHADER_STRING
(
 
 varying highp float k;
 uniform highp float N;
 uniform highp float s;
 
 void main()
 {
     highp vec4 result;
     result.r = cos(6.283185307*(k-0.5)/N);
     result.g = s*sin(6.283185307*(k-0.5)/N);
     result.ba = vec2(1.0, 1.0);
     gl_FragColor = result;
 }
);

ISTextureRef FFTSubBlock::renderPhaseTable(GLuint sigSize, int sign)
{
    
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glViewport(0, 0, sigSize, 64);
    ISTextureRef table = ISRe16Rgba<>::make(sigSize, 64);
    table->attachToFramebuffer(framebuffer);
    ISShaderProgram tableProgram;
    tableProgram.loadShader(phaseTableFragSource, phaseTableVertSource, {
        {0, "positionIn"},
        {1, "kIn"}
    });
    vector<GLfloat> vertices;
    makeGlAttributeColumnVarying(vertices, 64.0, 0, sigSize, {0.0}, {static_cast<GLfloat>(sigSize)});
    ISVertexArray vertexArray;
    vertexArray.addFloatAttribute(0, 3);
    vertexArray.addFloatAttribute(1, 1);
    vertexArray.upload(vertices);
    glUseProgram(tableProgram.program());
    GLuint orthoMatrixUP = glGetUniformLocation(tableProgram.program(), "orthoMatrix");
    GLuint nUP = glGetUniformLocation(tableProgram.program(), "N");
    GLuint sUP = glGetUniformLocation(tableProgram.program(), "s");
    GLKMatrix4 orthoMatrix = GLKMatrix4MakeOrtho(0.0, sigSize, 0.0, 64.0, -1.0, 1.0);
    glUniformMatrix4fv(orthoMatrixUP, 1, false, orthoMatrix.m);
    glUniform1f(sUP, static_cast<GLfloat>(sign));
    glUniform1f(nUP, static_cast<GLfloat>(sigSize));
    vertexArray.draw();
    table->detach();
    glDeleteFramebuffers(1, &framebuffer);
    _phaseTableCache.insert(pair<int, ISTextureRef>(sign*sigSize, table));
    return table;
}
void FFTSubBlock::preparePhaseTable(GLuint sigSize, int sign)
{
    DLPRINT("Looking for phase table %u\n", sigSize);
    auto it = _phaseTableCache.find(sign*sigSize);
    if (it == _phaseTableCache.end()) {
        DLPRINT("None found, making and caching a new one\n");
        renderPhaseTable(sigSize, sign);
    }
}
ISTextureRef FFTSubBlock::getPhaseTable()
{
    ISTextureRef table = NULL;
    auto it = _phaseTableCache.find(_sign*_sigSize);
    if (it != _phaseTableCache.end()) {
        table = it->second;
    } else {
        assert(false); //The table cache should be ready so this path doesn't execute. Call prepare phase table in the appropriate location
        table = renderPhaseTable(_sigSize, _sign);
    }
    return table;
}

void FFTSubBlock::bindPhaseTable(GLuint textureUnit)
{
    DLPRINT("Binding phase table\n");
    ISTextureRef table = getPhaseTable();
    table->bindToShader(_phaseCorrectTableUP, textureUnit);
}
void FFTSubBlock::releaseCache()
{
    DLPRINT("Releasing cached phase table textures");
    for (pair<GLuint, ISTextureRef> texture : _phaseTableCache) {
        texture.second->deleteTexture();
        delete texture.second;
    }
}