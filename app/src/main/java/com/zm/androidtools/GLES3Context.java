package com.zm.androidtools;

import android.opengl.EGL14;
import android.opengl.EGLExt;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLSurface;
import android.opengl.GLES31;

enum ShaderFrequence
{
    VertexShader,
    PixelShader,
    ComputeShader
}

class ShaderCompileResult
{
    public boolean bSuccess;
    public String result;
    public int shader;
}

class ShaderLinkResult
{
    public boolean bSuccess;
    public String result;
    public int program;
}
public class GLES3Context {
    private static final String TAG = "ShaderUtils";
    private EGLDisplay eglDisplay;
    private EGLContext eglContext;
    private EGLSurface eglSurface;

    public boolean init() {
        // 1. 获取 EGLDisplay
        eglDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY);
        int[] version = new int[2];
        EGL14.eglInitialize(eglDisplay, version, 0, version, 1);

        // 2. 配置支持 GLES3.0 + PBuffer Surface
        int[] configAttribs = {
                EGL14.EGL_RENDERABLE_TYPE, EGLExt.EGL_OPENGL_ES3_BIT_KHR,
                EGL14.EGL_SURFACE_TYPE, EGL14.EGL_PBUFFER_BIT,
                EGL14.EGL_RED_SIZE, 8,
                EGL14.EGL_GREEN_SIZE, 8,
                EGL14.EGL_BLUE_SIZE, 8,
                EGL14.EGL_ALPHA_SIZE, 8,
                EGL14.EGL_NONE
        };

        EGLConfig[] configs = new EGLConfig[1];
        int[] numConfigs = new int[1];

        if (!EGL14.eglChooseConfig(
                eglDisplay, configAttribs, 0,
                configs, 0, configs.length,
                numConfigs, 0)) {
            throw new RuntimeException("eglChooseConfig failed!");
        }

        // 3. 创建离屏 PBuffer（1x1 大小）
        int[] pbufferAttribs = {
                EGL14.EGL_WIDTH, 1,
                EGL14.EGL_HEIGHT, 1,
                EGL14.EGL_NONE
        };

        eglSurface = EGL14.eglCreatePbufferSurface(
                eglDisplay,
                configs[0],
                pbufferAttribs, 0);

        // 4. 创建 GLES 3 上下文
        int[] contextAttribs = {
                EGL14.EGL_CONTEXT_CLIENT_VERSION, 3,
                EGL14.EGL_NONE
        };

        eglContext = EGL14.eglCreateContext(
                eglDisplay,
                configs[0],
                EGL14.EGL_NO_CONTEXT,
                contextAttribs, 0);

        // 5. 绑定上下文
        if (!EGL14.eglMakeCurrent(
                eglDisplay,
                eglSurface,
                eglSurface,
                eglContext)) {
            throw new RuntimeException("eglMakeCurrent failed");
        }
        return true;
    }

    public void destroy() {
        EGL14.eglMakeCurrent(
                eglDisplay,
                EGL14.EGL_NO_SURFACE,
                EGL14.EGL_NO_SURFACE,
                EGL14.EGL_NO_CONTEXT);

        EGL14.eglDestroySurface(eglDisplay, eglSurface);
        EGL14.eglDestroyContext(eglDisplay, eglContext);
        EGL14.eglTerminate(eglDisplay);

        eglDisplay = null;
        eglSurface = null;
        eglContext = null;
    }

    public ShaderCompileResult compileShader(String source, ShaderFrequence frequence) {
        ShaderCompileResult result = new ShaderCompileResult();
        int shader = 0;
        if (frequence == ShaderFrequence.VertexShader) {
            shader = GLES31.glCreateShader(GLES31.GL_VERTEX_SHADER);
        } else if (frequence == ShaderFrequence.PixelShader) {
            shader = GLES31.glCreateShader(GLES31.GL_FRAGMENT_SHADER);
        } else if (frequence == ShaderFrequence.ComputeShader) {
            shader = GLES31.glCreateShader(GLES31.GL_COMPUTE_SHADER);
        } else {
            result.bSuccess = false;
            result.result = "glCreateShader frequence err";
            return result;
        }

        if (shader == 0) {
            result.bSuccess = false;
            result.result = "glCreateShader false";
            return result;
        }


        // Ensure source has no BOM at start:
        source = source.replaceFirst("^\\uFEFF", "");


        GLES31.glShaderSource(shader, source);
        GLES31.glCompileShader(shader);


        int[] status = new int[1];
        GLES31.glGetShaderiv(shader, GLES31.GL_COMPILE_STATUS, status, 0);


        int[] infoLen = new int[1];
        GLES31.glGetShaderiv(shader, GLES31.GL_INFO_LOG_LENGTH, infoLen, 0);


        StringBuilder sb = new StringBuilder();
        sb.append("GL_COMPILE_STATUS = ").append(status[0]).append("\n");
        sb.append("GL_INFO_LOG_LENGTH = ").append(infoLen[0]).append("\n");


        if (infoLen[0] > 1) {
            // Retrieve log
            String log = GLES31.glGetShaderInfoLog(shader);
            sb.append(log).append("\n");
        } else {
            sb.append("(no info log)\n");
        }

        result.bSuccess = status[0] != 0;
        result.result = sb.toString();
        result.shader = shader;

        if (!result.bSuccess) {
            deleteShader(shader);
            result.shader = 0;
        }
        return result;
    }

    public void deleteShader(int shader) {
        GLES31.glDeleteShader(shader);
    }

    public ShaderLinkResult linkProgram(int vertexShader, int fragmentShader) {
        ShaderLinkResult result = new ShaderLinkResult();
        int program = GLES31.glCreateProgram();
        if (program == 0)
        {
            result.bSuccess = false;
            result.result = "glCreateProgram false";
            return result;
        }
        GLES31.glAttachShader(program, vertexShader);
        GLES31.glAttachShader(program, fragmentShader);
        GLES31.glLinkProgram(program);

        // 检查 link 结果
        int[] linkStatus = new int[1];
        GLES31.glGetProgramiv(program, GLES31.GL_LINK_STATUS, linkStatus, 0);

        int[] infoLen = new int[1];
        GLES31.glGetProgramiv(program, GLES31.GL_INFO_LOG_LENGTH, infoLen, 0);

        StringBuilder sb = new StringBuilder();
        sb.append("GL_LINK_STATUS = ").append(linkStatus[0]).append("\n");
        sb.append("GL_INFO_LOG_LENGTH = ").append(infoLen[0]).append("\n");

        if (infoLen[0] > 1) {
            // Retrieve log
            String log = GLES31.glGetProgramInfoLog(program);
            sb.append(log).append("\n");
        } else {
            sb.append("(no info log)\n");
        }

        result.bSuccess = linkStatus[0] != 0;
        result.result = sb.toString();
        result.program = program;

        if (!result.bSuccess) {
            deleteProgram(program);
            result.program = 0;
        }
        return result;
    }

    public void deleteProgram(int program) {
        GLES31.glDeleteProgram(program);
    }
}
