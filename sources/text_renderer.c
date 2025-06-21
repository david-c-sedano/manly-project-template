FontRenderData
Load_TTF (const char* path, float scale)
{
    FontRenderData font_render_data;
    // set all the bboxes to zero real quick
    for (int i=0; i<128; i++) font_render_data.bboxes[i] = (Quad){0,0,0,0};

    long size;
    FILE* file = fopen(path, "rb");
    if (file == NULL) { 
        printf("Failed to acquire font - %s\n", path); 
        return (FontRenderData){ 0 };
    }
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char* buffer = malloc(size);
    fread(buffer, size, 1, file);
    fclose(file);

    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, buffer, 0)) {
        printf("Failed to acquire font - %s\n", path);
        return (FontRenderData){ 0 };
    }

    float font_scale = stbtt_ScaleForPixelHeight(&info, scale);

    const char* ascii_chars = 
    " !\"#$%&'()*+,-./"
    "0123456789"
    ":;<=>?@"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "[\\]^_`"
    "abcdefghijklmnopqrstuvwxyz"
    "{|}~";

    // this is freaking ridiculous
    int ascent, descent, linegap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &linegap);
    ascent = roundf(ascent * font_scale);
    descent = roundf(descent * font_scale);
    linegap = roundf(linegap * font_scale);

    int atlas_height = roundf((ascent - descent) + linegap);

    int atlas_width = 0;
    int len = strlen(ascii_chars);
    for (int i=0; i<len; i++)
    {
        int advance_width;
        int left_side_bearing;
        stbtt_GetCodepointHMetrics(&info, ascii_chars[i], &advance_width, &left_side_bearing);
   
        atlas_width += roundf(font_scale * advance_width);
    }

    unsigned char* atlas_data = calloc(atlas_width * atlas_height, sizeof(unsigned char));
    int x = 0;
    for (int i=0; i<strlen(ascii_chars); i++)
    {
        int advance_width;
        int left_side_bearing;
        stbtt_GetCodepointHMetrics(&info, ascii_chars[i], &advance_width, &left_side_bearing);

        int bb_x1, bb_y1, bb_x2, bb_y2;
        stbtt_GetCodepointBitmapBox(
            &info, 
            ascii_chars[i], 
            font_scale, 
            font_scale, 
            &bb_x1, &bb_y1, &bb_x2, &bb_y2
        );

        int char_width = bb_x2 - bb_x1;
        int char_height = bb_y2 - bb_y1;
        int y_offset = ascent + bb_y1;
        int offset = x + roundf(left_side_bearing * font_scale) + (y_offset * atlas_width);
        stbtt_MakeCodepointBitmap(
            &info, 
            atlas_data + offset, 
            char_width, 
            char_height, 
            atlas_width, 
            font_scale,
            font_scale,
            ascii_chars[i]
        );

        font_render_data.bboxes[ascii_chars[i]] = (Quad){
            x,
            x + roundf(advance_width * font_scale),
            0,
            1,
        };

        x += roundf(advance_width * font_scale);
    }

    font_render_data.gl_id = rlLoadTexture(
        atlas_data, 
        atlas_width,
        atlas_height,
        RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
        1
    );

    // for some reason, rlgl doesn't support alpha channel only textures
    // so I gotta dive into raw OpenGL to get the thing to work :(
    glBindTexture(GL_TEXTURE_2D, font_render_data.gl_id);
    GLint swizzle_mask[] = { GL_ONE, GL_ONE, GL_ONE, GL_RED };
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle_mask);

    // im not sure if rlgl does this by default
    rlTextureParameters(font_render_data.gl_id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_CLAMP);
    rlTextureParameters(font_render_data.gl_id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_CLAMP);

    font_render_data.atlas_width = atlas_width;
    font_render_data.atlas_height = atlas_height;
    font_render_data.atlas_data = atlas_data;
    free(buffer);

    return font_render_data;
}

void
DrawWords (FontRenderData font, const char* text, float x, float y, RGBA color)
{
    rlSetTexture(font.gl_id);
    rlBegin(RL_QUADS);
    rlColor4ub(color.r, color.g, color.b, color.a);

    int origin = x, len = strlen(text);
    for (int i=0; i<len; i++)
    {
        if (text[i] == '\n')
        {
            x = origin;
            y += font.atlas_height;
            continue;
        }
        else if (text[i] == '\t')
        {
            x += (font.bboxes[' '].right - font.bboxes[' '].left) * 4;
            continue;
        }

        Quad bbox = font.bboxes[text[i]];
        int width = bbox.right - bbox.left;
        int height = font.atlas_height;
        float texture_origin = bbox.left / (float)font.atlas_width;
        float texture_width = width / (float)font.atlas_width;

        rlTexCoord2f(texture_origin, 0);
        rlVertex2f(x, y);

        rlTexCoord2f(texture_origin, 1);
        rlVertex2f(x, y + height);

        rlTexCoord2f(texture_origin + texture_width, 1);
        rlVertex2f(x + width, y + height);

        rlTexCoord2f(texture_origin + texture_width, 0);
        rlVertex2f(x + width, y);

        x += width;
    }

    rlEnd();
    rlSetTexture(0);
}
// ~170 LINES JUST TO PRINT "Hello World!" TO A WINDOW
// THIS IS NOT COUNTING THE SINGLE HEADERS AND PLATFORM INIT CODE OF COURSE, EITHER WAY, IN YOU'RE ****ING FACE MODERN GRAPHICS PROGRAMMING!!!!

int
TextWidth (FontRenderData font, const char* text)
{
    int sum = 0, len = strlen(text);
    for (int i=0; i<len; i++)
    {
        Quad bbox = font.bboxes[text[i]];
        int width = bbox.right - bbox.left;
        sum += width;
    }
    return sum;
}

int TextHeight (FontRenderData font) { return font.atlas_height; }