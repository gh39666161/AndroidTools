package com.zm.androidtools;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;

import com.zm.androidtools.databinding.ActivityMainBinding;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'androidtools' library on application startup.
    static {
        System.loadLibrary("androidtools");
    }
    private static final String TAG = "MainActivity";

    // Name of the SPIR-V asset to load for the Vulkan shader-module test.
    // Add the .spv file under app/src/main/assets/ with this name.
    // SPIR-V assets to load for the Vulkan pipeline test. Add the .spv files
    // under app/src/main/assets/ with these names.
    private static final String SPV_VERT_ASSET_NAME = "MobileBasePassVertexShader.spv";
    private static final String SPV_FRAG_ASSET_NAME = "MobileBasePassPixelShader.spv";

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        binding.btnCompileShader.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                onCompileShaderClick();
            }
        });

        binding.btnCompileLinkShader.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                OnCompileLinkShaderClick();
            }
        });

        binding.btnCompileSpirv.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                onCompileSpirvClick();
            }
        });

        binding.tvLog.setText("");

        // Example of a call to a native method
//        TextView tv = binding.sampleText;
//        tv.setText(stringFromJNI());
    }


    private void onCompileShaderClick() {
        StringBuilder logStrBuilder = new StringBuilder();
        appendLog(logStrBuilder, "onCompileShaderClick");

        new Thread(() -> {
            GLES3Context gles3 = new GLES3Context();
            if (!gles3.init()) {
                appendLog(logStrBuilder, "gles3 init fail");
                updateLogView(logStrBuilder);
                return;
            }
            try {
                String shaderSource = readAssetText("Output.frag");
                ShaderCompileResult result = gles3.compileShader(shaderSource, ShaderFrequence.PixelShader);
                if (result.bSuccess)
                {
                    gles3.deleteShader(result.shader);
                }

                appendLog(logStrBuilder, "Compile shader result:\n" + result.result);
                updateLogView(logStrBuilder);
            } catch (Exception e) {
                final String msg = "Exception: " + e.toString();
                Log.e(TAG, msg);
                runOnUiThread(() -> {
                    binding.tvLog.setText(msg);
                });
            }
            gles3.destroy();
        }).start();

    }

    private void OnCompileLinkShaderClick() {
        StringBuilder logStrBuilder = new StringBuilder();
        appendLog(logStrBuilder, "onCompileLinkShaderClick");

        new Thread(() -> {
            GLES3Context gles3 = new GLES3Context();
            if (!gles3.init()) {
                appendLog(logStrBuilder, "gles3 init fail");
                updateLogView(logStrBuilder);
                return;
            }
            try {
                String shaderSource = readAssetText("Output.vert");
                ShaderCompileResult vertResult = gles3.compileShader(shaderSource, ShaderFrequence.VertexShader);
                appendLog(logStrBuilder, "Compile vertex shader result:\n" + vertResult.result);
                if (!vertResult.bSuccess)
                {
                    updateLogView(logStrBuilder);
                    return;
                }

                shaderSource = readAssetText("Output.frag");
                ShaderCompileResult fragResult = gles3.compileShader(shaderSource, ShaderFrequence.PixelShader);
                appendLog(logStrBuilder, "Compile frag shader result:\n" + fragResult.result);
                if (!fragResult.bSuccess)
                {
                    updateLogView(logStrBuilder);
                    gles3.deleteShader(vertResult.shader);
                    return;
                }
                ShaderLinkResult linkResult = gles3.linkProgram(vertResult.shader, fragResult.shader);
                appendLog(logStrBuilder, "Link shader program result:\n" + linkResult.result);
                gles3.deleteShader(vertResult.shader);
                gles3.deleteShader(fragResult.shader);

                if (linkResult.bSuccess) {
                    gles3.deleteProgram(linkResult.program);
                }

                updateLogView(logStrBuilder);
            } catch (Exception e) {
                final String msg = "Exception: " + e.toString();
                Log.e(TAG, msg);
                runOnUiThread(() -> {
                    binding.tvLog.setText(msg);
                });
            }
            gles3.destroy();
        }).start();
    }

    private void onCompileSpirvClick() {
        StringBuilder logStrBuilder = new StringBuilder();
        appendLog(logStrBuilder, "onCompileSpirvClick");

        new Thread(() -> {
            VulkanContext vulkan = new VulkanContext();
            try {
                String initLog = vulkan.init();
                appendLog(logStrBuilder, "Vulkan init:\n" + initLog);

                if (!vulkan.isInitialized()) {
                    updateLogView(logStrBuilder);
                    return;
                }

                byte[] vertSpirv;
                byte[] fragSpirv;
                try {
                    vertSpirv = readAssetBytes(SPV_VERT_ASSET_NAME);
                    fragSpirv = readAssetBytes(SPV_FRAG_ASSET_NAME);
                } catch (Exception e) {
                    appendLog(logStrBuilder, "Failed to load spv assets: " + e
                            + "\nAdd .spv files at app/src/main/assets/"
                            + SPV_VERT_ASSET_NAME + " and " + SPV_FRAG_ASSET_NAME);
                    updateLogView(logStrBuilder);
                    return;
                }

                appendLog(logStrBuilder, "Loaded vertex spv: " + vertSpirv.length
                        + " bytes (" + SPV_VERT_ASSET_NAME + ")\n"
                        + "Loaded fragment spv: " + fragSpirv.length
                        + " bytes (" + SPV_FRAG_ASSET_NAME + ")");

                String pipelineLog = vulkan.createGraphicsPipeline(vertSpirv, fragSpirv);
                appendLog(logStrBuilder, "createGraphicsPipeline:\n" + pipelineLog);

                updateLogView(logStrBuilder);
            } catch (Exception e) {
                final String msg = "Exception: " + e.toString();
                Log.e(TAG, msg);
                runOnUiThread(() -> binding.tvLog.setText(msg));
            } finally {
                vulkan.destroy();
            }
        }).start();
    }

    /**
     * Appends a message to the on-screen log builder and mirrors it to logcat.
     */
    private void appendLog(StringBuilder builder, String message) {
        Log.i(TAG, message);
        builder.append(message).append('\n');
    }

    /** Pushes the current log builder contents to the on-screen TextView. */
    private void updateLogView(StringBuilder builder) {
        final String text = builder.toString();
        runOnUiThread(() -> binding.tvLog.setText(text));
    }

    private String readAssetText(String name) throws Exception {
        InputStream is = getAssets().open(name);
        BufferedReader br = new BufferedReader(new InputStreamReader(is));
        StringBuilder sb = new StringBuilder();
        String line;
        while ((line = br.readLine()) != null) {
            sb.append(line).append('\n');
        }
        br.close();
        return sb.toString();
    }

    private byte[] readAssetBytes(String name) throws Exception {
        InputStream is = getAssets().open(name);
        try {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            byte[] buf = new byte[8192];
            int n;
            while ((n = is.read(buf)) != -1) {
                bos.write(buf, 0, n);
            }
            return bos.toByteArray();
        } finally {
            is.close();
        }
    }

    /**
     * A native method that is implemented by the 'androidtools' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}