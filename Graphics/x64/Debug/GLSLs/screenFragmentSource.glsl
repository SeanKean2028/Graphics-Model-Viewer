        #version 330 core
        in vec2 Texcoord;
        out vec4 outColor;
        uniform sampler2D texFramebuffer;
        uniform int selector;
        
        const float blurSizeH = 1.0 / 300.0;
        const float blurSizeV = 1.0 / 200.0;

        void main()
        {
            if (selector == 1) {
                // Greyscale
                vec4 c = texture(texFramebuffer, Texcoord);
                float avg = (c.r + c.g + c.b) / 3.0;
                outColor = vec4(avg, avg, avg, 1.0);
            } 
            else if (selector == 2) {
                // Simple blur
                vec4 sum = vec4(0.0);
                for (int y = -4; y <= 4; ++y) {
                    for (int x = -4; x <= 4; ++x) {
                        sum += texture(texFramebuffer,
                                       Texcoord + vec2(float(x) * blurSizeH,
                                                       float(y) * blurSizeV));
                    }
                }
                outColor = sum / (9.0 * 9.0);
            } 
            else {
                // Default: just pass through
                outColor = texture(texFramebuffer, Texcoord);
            }
        }