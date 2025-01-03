/*
SDL_FontCache: A font cache for SDL and SDL_ttf
by Jonathan Dearborn

See SDL_FontCache.h for license info.
*/

#include "SDL_FontCache.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Visual C does not support static inline
#ifndef static_inline
	#ifdef _MSC_VER
		#define static_inline static
	#else
		#define static_inline static inline
	#endif
#endif

#if SDL_VERSION_ATLEAST(2,0,0)
    #define FC_GET_ALPHA(sdl_color) ((sdl_color).a)
#else
    #define FC_GET_ALPHA(sdl_color) ((sdl_color).unused)
#endif

// Need SDL_RenderIsClipEnabled() for proper clipping support
#if SDL_VERSION_ATLEAST(2,0,4)
    #define ENABLE_SDL_CLIPPING
#endif

#define FC_MIN(a,b) ((a) < (b)? (a) : (b))
#define FC_MAX(a,b) ((a) > (b)? (a) : (b))


// vsnprintf replacement from Valentin Milea:
// http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
#if defined(_MSC_VER) && _MSC_VER < 1900

#define snprintf c99_snprintf
#define vsnprintf c99_vsnprintf

__inline int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}

__inline int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(outBuf, size, format, ap);
    va_end(ap);

    return count;
}

#endif


#define FC_EXTRACT_VARARGS(buffer, start_args) \
{ \
    va_list lst; \
    va_start(lst, start_args); \
    vsnprintf(buffer, fc_buffer_size, start_args, lst); \
    va_end(lst); \
}

// Extra pixels of padding around each glyph to avoid linear filtering artifacts
#define FC_CACHE_PADDING 1



static Uint8 has_clip(FC_Target* dest)
{
    #if defined(ENABLE_SDL_CLIPPING)
    return (Uint8)SDL_RenderClipEnabled(dest);
    #else
    return 0;
    #endif
}

static FC_Rect get_clip(FC_Target* dest)
{
    #if defined(ENABLE_SDL_CLIPPING)
    SDL_Rect r;
    SDL_GetRenderClipRect(dest, &r);
    FC_Rect fr = {(float)r.x, (float)r.y, (float)r.w, (float)r.h}; //frfr no cap
    return fr;
    #else
    FC_Rect r = {0.0f, 0.0f, 0.0f, 0.0f};
    return r;
    #endif
}

static void set_clip(FC_Target* dest, FC_Rect* rect)
{
    #if defined(ENABLE_SDL_CLIPPING)
    const SDL_Rect ri = {(int)rect->x, (int)rect->y, (int)rect->w, (int)rect->h};
    SDL_SetRenderClipRect(dest, &ri);
    #endif
}

static void set_color(FC_Image* src, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetTextureColorMod(src, r, g, b);
    SDL_SetTextureAlphaMod(src, a);
}



static char* new_concat(const char* a, const char* b)
{
    // Create new buffer
    unsigned int len_a = (unsigned int)strlen(a);
    char* new_string = (char*)malloc(len_a + strlen(b) + 1);

    // Concatenate strings in the new buffer
    strcpy(new_string, a);
    strcpy(new_string + len_a, b);

    return new_string;
}

static char* replace_concat(char** a, const char* b)
{
    char* new_string = new_concat(*a, b);
    free(*a);
    *a = new_string;
    return *a;
}


// Width of a tab in units of the space width (sorry, no tab alignment!)
static unsigned int fc_tab_width = 4;

// Shared buffer for variadic text
static char* fc_buffer = NULL;
static unsigned int fc_buffer_size = 1024;

// The number of fonts that has been created but not freed
static int NUM_EXISTING_FONTS = 0;

// Globals for GetString functions
static char* ASCII_STRING = NULL;

char* FC_GetStringASCII(void)
{
    if(ASCII_STRING == NULL)
    {
        int i;
        char c;
        ASCII_STRING = (char*)malloc(512);
        memset(ASCII_STRING, 0, 512);
        i = 0;
        c = 32;
        while(1)
        {
            ASCII_STRING[i] = c;
            if(c == 126)
                break;
            ++i;
            ++c;
        }
    }
    return U8_strdup(ASCII_STRING);
}

FC_Rect FC_MakeRect(float x, float y, float w, float h)
{
    FC_Rect r = {x, y, w, h};
    return r;
}

FC_Scale FC_MakeScale(float x, float y)
{
    FC_Scale s = {x, y};

    return s;
}

SDL_Color FC_MakeColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_Color c = {r, g, b, a};

    return c;
}

FC_Effect FC_MakeEffect(FC_AlignEnum alignment, FC_Scale scale, SDL_Color color)
{
    FC_Effect e;

    e.alignment = alignment;
    e.scale = scale;
    e.color = color;

    return e;
}

FC_GlyphData FC_MakeGlyphData(int cache_level, Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
    FC_GlyphData gd;

    gd.rect.x = x;
    gd.rect.y = y;
    gd.rect.w = w;
    gd.rect.h = h;
    gd.cache_level = cache_level;

    return gd;
}

// Enough to hold all of the ascii characters and some.
#define FC_DEFAULT_NUM_BUCKETS 300

typedef struct FC_MapNode
{
    Uint32 key;
    FC_GlyphData value;
    struct FC_MapNode* next;

} FC_MapNode;

typedef struct FC_Map
{
    int num_buckets;
    FC_MapNode** buckets;
} FC_Map;



static FC_Map* FC_MapCreate(int num_buckets)
{
    int i;
    FC_Map* map = (FC_Map*)malloc(sizeof(FC_Map));

    map->num_buckets = num_buckets;
    map->buckets = (FC_MapNode**)malloc((size_t)num_buckets * sizeof(FC_MapNode*));

    for(i = 0; i < num_buckets; ++i)
    {
        map->buckets[i] = NULL;
    }

    return map;
}

/*static void FC_MapClear(FC_Map* map)
{
    int i;
    if(map == NULL)
        return;

    // Go through each bucket
    for(i = 0; i < map->num_buckets; ++i)
    {
        // Delete the nodes in order
        FC_MapNode* node = map->buckets[i];
        while(node != NULL)
        {
            FC_MapNode* last = node;
            node = node->next;
            free(last);
        }
        // Set the bucket to empty
        map->buckets[i] = NULL;
    }
}*/

static void FC_MapFree(FC_Map* map)
{
    int i;
    if(map == NULL)
        return;

    // Go through each bucket
    for(i = 0; i < map->num_buckets; ++i)
    {
        // Delete the nodes in order
        FC_MapNode* node = map->buckets[i];
        while(node != NULL)
        {
            FC_MapNode* last = node;
            node = node->next;
            free(last);
        }
    }

    free(map->buckets);
    free(map);
}

// Note: Does not handle duplicates in any special way.
static FC_GlyphData* FC_MapInsert(FC_Map* map, Uint32 codepoint, FC_GlyphData glyph)
{
    Uint32 index;
    FC_MapNode* node;
    if(map == NULL)
        return NULL;

    // Get index for bucket
    index = codepoint % (Uint32)(map->num_buckets);

    // If this bucket is empty, create a node and return its value
    if(map->buckets[index] == NULL)
    {
        node = map->buckets[index] = (FC_MapNode*)malloc(sizeof(FC_MapNode));
        node->key = codepoint;
        node->value = glyph;
        node->next = NULL;
        return &node->value;
    }

    for(node = map->buckets[index]; node != NULL; node = node->next)
    {
        // Find empty node and add a new one on.
        if(node->next == NULL)
        {
            node->next = (FC_MapNode*)malloc(sizeof(FC_MapNode));
            node = node->next;

            node->key = codepoint;
            node->value = glyph;
            node->next = NULL;
            return &node->value;
        }
    }

    return NULL;
}

static FC_GlyphData* FC_MapFind(FC_Map* map, Uint32 codepoint)
{
    Uint32 index;
    FC_MapNode* node;
    if(map == NULL)
        return NULL;

    // Get index for bucket
    index = codepoint % (Uint32)(map->num_buckets);

    // Go through list until we find a match
    for(node = map->buckets[index]; node != NULL; node = node->next)
    {
        if(node->key == codepoint)
            return &node->value;
    }

    return NULL;
}



struct FC_Font
{
    #ifndef FC_USE_SDL_GPU
    SDL_Renderer* renderer;
    #endif

    TTF_Font* ttf_source;  // TTF_Font source of characters
    Uint8 owns_ttf_source;  // Can we delete the TTF_Font ourselves?

    FC_FilterEnum filter;

    SDL_Color default_color;
    Uint16 height;

    Uint16 maxWidth;
    Uint16 baseline;
    float ascent;
    float descent;

    int lineSpacing;
    int letterSpacing;

    // Uses 32-bit (4-byte) Unicode codepoints to refer to each glyph
    // Codepoints are little endian (reversed from UTF-8) so that something like 0x00000005 is ASCII 5 and the map can be indexed by ASCII values
    FC_Map* glyphs;

    FC_GlyphData last_glyph;  // Texture packing cursor
    int glyph_cache_size;
    int glyph_cache_count;
    FC_Image** glyph_cache;

    char* loading_string;

};

// Private
static FC_GlyphData* FC_PackGlyphData(FC_Font* font, Uint32 codepoint, Uint16 width, Uint16 maxWidth, Uint16 maxHeight);


static FC_Rect FC_RenderLeft(FC_Font* font, FC_Target* dest, float x, float y, FC_Scale scale, const char* text);
static FC_Rect FC_RenderCenter(FC_Font* font, FC_Target* dest, float x, float y, FC_Scale scale, const char* text);
static FC_Rect FC_RenderRight(FC_Font* font, FC_Target* dest, float x, float y, FC_Scale scale, const char* text);


static_inline SDL_Surface* FC_CreateSurface32(Uint32 width, Uint32 height)
{
    return SDL_CreateSurface((int)width, (int)height, SDL_PIXELFORMAT_RGBA8888);
}


char* U8_alloc(unsigned int size)
{
    char* result = NULL;
    if(size == 0)
        return NULL;

    result = (char*)malloc(size);
    result[0] = '\0';

    return result;
}

void U8_free(char* string)
{
    free(string);
}

char* U8_strdup(const char* string)
{
    char* result;
    if(string == NULL)
        return NULL;

    result = (char*)malloc(strlen(string)+1);
    strcpy(result, string);

    return result;
}

int U8_strlen(const char* string)
{
    int length = 0;
    if(string == NULL)
        return 0;

    while(*string != '\0')
    {
        string = U8_next(string);
        ++length;
    }

    return length;
}

int U8_charsize(const char* character)
{
    if(character == NULL)
        return 0;

    if((unsigned char)*character <= 0x7F)
        return 1;
    else if((unsigned char)*character < 0xE0)
        return 2;
    else if((unsigned char)*character < 0xF0)
        return 3;
    else
        return 4;
    return 1;
}

int U8_charcpy(char* buffer, const char* source, int buffer_size)
{
    int charsize;
    if(buffer == NULL || source == NULL || buffer_size < 1)
        return 0;

    charsize = U8_charsize(source);
    if(charsize > buffer_size)
        return 0;

    memcpy(buffer, source, (size_t)charsize);
    return charsize;
}

const char* U8_next(const char* string)
{
    return string + U8_charsize(string);
}

int U8_strinsert(char* string, int position, const char* source, int max_bytes)
{
    int pos_u8char;
    int len;
    int add_len;
    int ulen;
    const char* string_start = string;

    if(string == NULL || source == NULL)
        return 0;

    len = (int)strlen(string);
    add_len = (int)strlen(source);
    ulen = U8_strlen(string);

    if(position == -1)
        position = ulen;

    if(position < 0 || position > ulen || len + add_len + 1 > max_bytes)
        return 0;

    // Move string pointer to the proper position
    pos_u8char = 0;
    while(*string != '\0' && pos_u8char < position)
    {
        string = (char*)U8_next(string);
        ++pos_u8char;
    }

    // Move the rest of the string out of the way
    memmove(string + add_len, string, (size_t)(len - (string - string_start) + 1));

    // Copy in the new characters
    memcpy(string, source, (size_t)add_len);

    return 1;
}

void U8_strdel(char* string, int position)
{
    if(string == NULL || position < 0)
        return;

    while(*string != '\0')
    {
        if(position == 0)
        {
            int chars_to_erase = U8_charsize(string);
            int remaining_bytes = (int)(strlen(string) + 1);
            memmove(string, string + chars_to_erase, (size_t)remaining_bytes);
            break;
        }

        string = (char*)U8_next(string);
        --position;
    }
}


static_inline FC_Rect FC_RectUnion(FC_Rect A, FC_Rect B)
{
    float x,x2,y,y2;
    x = FC_MIN(A.x, B.x);
    y = FC_MIN(A.y, B.y);
    x2 = FC_MAX(A.x+A.w, B.x+B.w);
    y2 = FC_MAX(A.y+A.h, B.y+B.h);
    {
        FC_Rect result = {x, y, FC_MAX(0, x2 - x), FC_MAX(0, y2 - y)};
        return result;
    }
}


FC_Rect FC_DefaultRenderCallback(FC_Image* src, FC_Rect* srcrect, FC_Target* dest, float x, float y, float xscale, float yscale)
{
    float w = srcrect->w * xscale;
    float h = srcrect->h * yscale;
    FC_Rect result;

    // FIXME: Why does the scaled offset look so wrong?
    SDL_FlipMode flip = SDL_FLIP_NONE;
    if(xscale < 0)
    {
        xscale = -xscale;
        flip = (SDL_FlipMode) ((int)flip | (int)SDL_FLIP_HORIZONTAL);
    }
    if(yscale < 0)
    {
        yscale = -yscale;
        flip = (SDL_FlipMode) ((int)flip | (int)SDL_FLIP_VERTICAL);
    }

    SDL_FRect r = { 
        (float)srcrect->x, 
        (float)srcrect->y, 
        (float)srcrect->w,
        (float)srcrect->h
    };
    SDL_FRect dr = {(float)x, (float)y, (float)(xscale*r.w), (float)(yscale*r.h)};
    SDL_RenderTextureRotated(dest, src, &r, &dr, 0, NULL, flip);

    result.x = x;
    result.y = y;
    result.w = w;
    result.h = h;
    return result;
}

static FC_Rect (*fc_render_callback)(FC_Image* src, FC_Rect* srcrect, FC_Target* dest, float x, float y, float xscale, float yscale) = &FC_DefaultRenderCallback;

void FC_SetRenderCallback(FC_Rect (*callback)(FC_Image* src, FC_Rect* srcrect, FC_Target* dest, float x, float y, float xscale, float yscale))
{
    if(callback == NULL)
        fc_render_callback = &FC_DefaultRenderCallback;
    else
        fc_render_callback = callback;
}

void FC_GetUTF8FromCodepoint(char* result, Uint32 codepoint)
{
    char a, b, c, d;

    if(result == NULL)
        return;

    a = (char)((codepoint >> 24) & 0xFF);
    b = (char)((codepoint >> 16) & 0xFF);
    c = (char)((codepoint >> 8) & 0xFF);
    d = (char)(codepoint & 0xFF);

    if(a == 0)
    {
        if(b == 0)
        {
            if(c == 0)
            {
                result[0] = d;
                result[1] = '\0';
            }
            else
            {
                result[0] = c;
                result[1] = d;
                result[2] = '\0';
            }
        }
        else
        {
            result[0] = b;
            result[1] = c;
            result[2] = d;
            result[3] = '\0';
        }
    }
    else
    {
        result[0] = a;
        result[1] = b;
        result[2] = c;
        result[3] = d;
        result[4] = '\0';
    }
}

Uint32 FC_GetCodepointFromUTF8(const char** c, Uint8 advance_pointer)
{
    Uint32 result = 0;
    const char* str;
    if(c == NULL || *c == NULL)
        return 0;

    str = *c;
    if((unsigned char)*str <= 0x7F)
        result = (Uint32)(*str);
    else if((unsigned char)*str < 0xE0)
    {
        result |= (Uint32)((unsigned char)(*str) << 8);
        result |= (unsigned char)(*(str+1));
        if(advance_pointer)
            *c += 1;
    }
    else if((unsigned char)*str < 0xF0)
    {
        result |= (Uint32)((unsigned char)(*str) << 16);
        result |= (Uint32)((unsigned char)(*(str+1)) << 8);
        result |= (unsigned char)(*(str+2));
        if(advance_pointer)
            *c += 2;
    }
    else
    {
        result |= (Uint32)((unsigned char)(*str) << 24);
        result |= (Uint32)((unsigned char)(*(str+1)) << 16);
        result |= (Uint32)((unsigned char)(*(str+2)) << 8);
        result |= (unsigned char)(*(str+3));
        if(advance_pointer)
            *c += 3;
    }
    return result;
}


void FC_SetLoadingString(FC_Font* font, const char* string)
{
    if(font == NULL)
        return;

    free(font->loading_string);
    font->loading_string = U8_strdup(string);
}


unsigned int FC_GetBufferSize(void)
{
    return fc_buffer_size;
}

void FC_SetBufferSize(unsigned int size)
{
    free(fc_buffer);
    if(size > 0)
    {
        fc_buffer_size = size;
        fc_buffer = (char*)malloc(fc_buffer_size);
    }
    else
        fc_buffer = (char*)malloc(fc_buffer_size);
}


unsigned int FC_GetTabWidth(void)
{
    return fc_tab_width;
}

void FC_SetTabWidth(unsigned int width_in_spaces)
{
    fc_tab_width = width_in_spaces;
}



// Constructors

static void FC_Init(FC_Font* font)
{
    if(font == NULL)
        return;

    #ifndef FC_USE_SDL_GPU
    font->renderer = NULL;
    #endif

    font->ttf_source = NULL;
    font->owns_ttf_source = 0;

    font->filter = FC_FILTER_NEAREST;

    font->default_color.r = 0;
    font->default_color.g = 0;
    font->default_color.b = 0;
    FC_GET_ALPHA(font->default_color) = 255;

    font->height = 0; // ascent+descent

    font->maxWidth = 0;
    font->baseline = 0;
    font->ascent = 0;
    font->descent = 0;

    font->lineSpacing = 0;
    font->letterSpacing = 0;

    // Give a little offset for when filtering/mipmaps are used.  Depending on mipmap level, this will still not be enough.
    font->last_glyph.rect.x = FC_CACHE_PADDING;
    font->last_glyph.rect.y = FC_CACHE_PADDING;
    font->last_glyph.rect.w = 0;
    font->last_glyph.rect.h = 0;
    font->last_glyph.cache_level = 0;

    if(font->glyphs != NULL)
        FC_MapFree(font->glyphs);

    font->glyphs = FC_MapCreate(FC_DEFAULT_NUM_BUCKETS);

    font->glyph_cache_size = 3;
    font->glyph_cache_count = 0;


    font->glyph_cache = (FC_Image**)malloc((size_t)font->glyph_cache_size * sizeof(FC_Image*));

	if (font->loading_string == NULL)
		font->loading_string = FC_GetStringASCII();
    //FC_Log("%s\n",font->loading_string);

    if(fc_buffer == NULL)
        fc_buffer = (char*)malloc(fc_buffer_size);
}

static Uint8 FC_GrowGlyphCache(FC_Font* font)
{
    if(font == NULL)
        return 0;
    SDL_Texture* new_level = SDL_CreateTexture(font->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, font->height * 12, font->height * 12);
    if(new_level == NULL || !FC_SetGlyphCacheLevel(font, font->glyph_cache_count, new_level))
    {
        FC_Log("Error: SDL_FontCache ran out of packing space and could not add another cache level.\n");
        SDL_DestroyTexture(new_level);
        return 0;
    }
    // bug: we do not have the correct color here, this might be the wrong color!
    //      , most functions use set_color_for_all_caches()
    //   - for evading this bug, you must use FC_SetDefaultColor(), before using any draw functions
    set_color(new_level, font->default_color.r, font->default_color.g, font->default_color.b, FC_GET_ALPHA(font->default_color));
    Uint8 r, g, b, a;
    SDL_Texture* prev_target = SDL_GetRenderTarget(font->renderer);
    SDL_FRect prev_clip;
    SDL_Rect prev_viewport;
    int prev_logicalw = 0, prev_logicalh = 0;
    Uint8 prev_clip_enabled;
    float prev_scalex, prev_scaley;
    // only backup if previous target existed (SDL will preserve them for the default target)
    if (prev_target) {
        prev_clip_enabled = has_clip(font->renderer);
        if (prev_clip_enabled)
            prev_clip = get_clip(font->renderer);
        SDL_GetRenderViewport(font->renderer, &prev_viewport);
        SDL_GetRenderScale(font->renderer, &prev_scalex, &prev_scaley);
        SDL_GetRenderLogicalPresentation(font->renderer, &prev_logicalw, &prev_logicalh, NULL);
    }
    SDL_SetTextureBlendMode(new_level, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(font->renderer, new_level);
    SDL_GetRenderDrawColor(font->renderer, &r, &g, &b, &a);
    SDL_SetRenderDrawColor(font->renderer, 0, 0, 0, 0);
    SDL_RenderClear(font->renderer);
    SDL_SetRenderDrawColor(font->renderer, r, g, b, a);
    SDL_SetRenderTarget(font->renderer, prev_target);
    if (prev_target) {
        if (prev_clip_enabled)
            set_clip(font->renderer, &prev_clip);
        if (prev_logicalw && prev_logicalh)
            SDL_SetRenderLogicalPresentation(
                font->renderer,
                prev_logicalw, 
                prev_logicalh,
                SDL_LOGICAL_PRESENTATION_DISABLED);
        else {
            SDL_SetRenderViewport(font->renderer, &prev_viewport);
            SDL_SetRenderScale(font->renderer, prev_scalex, prev_scaley);
        }
    }
    return 1;
}

Uint8 FC_UploadGlyphCache(FC_Font* font, int cache_level, SDL_Surface* data_surface)
{
    if(font == NULL || data_surface == NULL)
        return 0;
    SDL_Texture* new_level;
    
    {
        // Must upload with render target enabled so we can put more glyphs on later
        SDL_Renderer* renderer = font->renderer;

        new_level = SDL_CreateTexture(renderer, data_surface->format, SDL_TEXTUREACCESS_TARGET, data_surface->w, data_surface->h);
        SDL_SetTextureBlendMode(new_level, SDL_BLENDMODE_BLEND);

        Uint8 r, g, b, a;
        SDL_Texture* temp = SDL_CreateTextureFromSurface(renderer, data_surface);
        SDL_Texture* prev_target = SDL_GetRenderTarget(renderer);
        SDL_FRect prev_clip;
        SDL_Rect prev_viewport;
        int prev_logicalw, prev_logicalh;
        Uint8 prev_clip_enabled;
        float prev_scalex, prev_scaley;
        // only backup if previous target existed (SDL will preserve them for the default target)
        if (prev_target) {
            prev_clip_enabled = has_clip(renderer);
            if (prev_clip_enabled)
                prev_clip = get_clip(renderer);
            SDL_GetRenderViewport(renderer, &prev_viewport);
            SDL_GetRenderScale(renderer, &prev_scalex, &prev_scaley);
            SDL_GetRenderLogicalPresentation(renderer, &prev_logicalw, &prev_logicalh, NULL);
        }
        SDL_SetTextureBlendMode(temp, SDL_BLENDMODE_NONE);
        SDL_SetRenderTarget(renderer, new_level);

        SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        SDL_RenderTextureRotated(renderer, temp, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
        SDL_SetRenderTarget(renderer, prev_target);
        if (prev_target) {
            if (prev_clip_enabled)
                set_clip(renderer, &prev_clip);
            if (prev_logicalw && prev_logicalh)
                SDL_SetRenderLogicalPresentation(renderer, prev_logicalw, prev_logicalh,
                    SDL_LOGICAL_PRESENTATION_DISABLED);
            else {
                SDL_SetRenderViewport(renderer, &prev_viewport);
                SDL_SetRenderScale(renderer, prev_scalex, prev_scaley);
            }
        }

        SDL_DestroyTexture(temp);
    }
    if(new_level == NULL || !FC_SetGlyphCacheLevel(font, cache_level, new_level))
    {
        FC_Log("Error: SDL_FontCache ran out of packing space and could not add another cache level.\n");
        SDL_DestroyTexture(new_level);
        return 0;
    }
    return 1;
}

static FC_GlyphData* FC_PackGlyphData(FC_Font* font, Uint32 codepoint, Uint16 width, Uint16 maxWidth, Uint16 maxHeight)
{
    FC_Map* glyphs = font->glyphs;
    FC_GlyphData* last_glyph = &font->last_glyph;
    Uint16 height = font->height + FC_CACHE_PADDING;

    // TAB is special!
    if(codepoint == '\t')
    {
        FC_GlyphData spaceGlyph;
        FC_GetGlyphData(font, &spaceGlyph, ' ');
        width = (Uint16)((float)fc_tab_width * spaceGlyph.rect.w);
    }

    if(last_glyph->rect.x + last_glyph->rect.w + width >= maxWidth - FC_CACHE_PADDING)
    {
        if(last_glyph->rect.y + height + height >= maxHeight - FC_CACHE_PADDING)
        {
            // Get ready to pack on the next cache level when it is ready
            last_glyph->cache_level = font->glyph_cache_count;
            last_glyph->rect.x = FC_CACHE_PADDING;
            last_glyph->rect.y = FC_CACHE_PADDING;
            last_glyph->rect.w = 0;
            return NULL;
        }
        else
        {
            // Go to next row
            last_glyph->rect.x = FC_CACHE_PADDING;
            last_glyph->rect.y += height;
            last_glyph->rect.w = 0;
        }
    }

    // Move to next space
    last_glyph->rect.x += last_glyph->rect.w + 1 + FC_CACHE_PADDING;
    last_glyph->rect.w = width;

    return FC_MapInsert(glyphs, codepoint, FC_MakeGlyphData(last_glyph->cache_level, (Sint16)last_glyph->rect.x, (Sint16)last_glyph->rect.y, (Uint16)last_glyph->rect.w, (Uint16)last_glyph->rect.h));
}


FC_Image* FC_GetGlyphCacheLevel(FC_Font* font, int cache_level)
{
    if(font == NULL || cache_level < 0 || cache_level > font->glyph_cache_count)
        return NULL;

    return font->glyph_cache[cache_level];
}

Uint8 FC_SetGlyphCacheLevel(FC_Font* font, int cache_level, FC_Image* cache_texture)
{
    if(font == NULL || cache_level < 0)
        return 0;

    // Must be sequentially added
    if(cache_level > font->glyph_cache_count + 1)
        return 0;

    if(cache_level == font->glyph_cache_count)
    {
        font->glyph_cache_count++;

        // Grow cache?
        if(font->glyph_cache_count > font->glyph_cache_size)
        {
            // Copy old cache to new one
            int i;
            FC_Image** new_cache;
            new_cache = (FC_Image**)malloc((size_t)font->glyph_cache_count * sizeof(FC_Image*));
            for(i = 0; i < font->glyph_cache_size; ++i)
                new_cache[i] = font->glyph_cache[i];

            // Save new cache
            free(font->glyph_cache);
            font->glyph_cache_size = font->glyph_cache_count;
            font->glyph_cache = new_cache;
        }
    }

    font->glyph_cache[cache_level] = cache_texture;
    return 1;
}


FC_Font* FC_CreateFont(void)
{
    FC_Font* font;

    font = (FC_Font*)malloc(sizeof(FC_Font));
    memset(font, 0, sizeof(FC_Font));

    FC_Init(font);
    ++NUM_EXISTING_FONTS;

    return font;
}


// Assume this many will be enough...
#define FC_LOAD_MAX_SURFACES 10

Uint8 FC_LoadFontFromTTF(FC_Font* font, SDL_Renderer* renderer, TTF_Font* ttf, SDL_Color color)
{
    if(font == NULL || ttf == NULL)
        return 0;
    if(renderer == NULL)
        return 0;

    FC_ClearFont(font);

    font->renderer = renderer;

    font->ttf_source = ttf;

    //font->line_height = TTF_FontLineSkip(ttf);
    font->height = (Uint16)TTF_GetFontHeight(ttf);
    font->ascent = (float)TTF_GetFontAscent(ttf);
    font->descent = (float)(-TTF_GetFontDescent(ttf));

    // Some bug for certain fonts can result in an incorrect height.
    if(font->height < font->ascent - font->descent)
        font->height = (Uint16)(font->ascent - font->descent);

    font->baseline = (Uint16)(font->height - font->descent);

    font->default_color = color;

    {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* glyph_surf;
        char buff[5];
        const char* buff_ptr = buff;
        const char* source_string;
        Uint8 packed = 0;

        // Copy glyphs from the surface to the font texture and store the position data
        // Pack row by row into a square texture
        // Try figuring out dimensions that make sense for the font size.
        unsigned int w = font->height*12;
        unsigned int h = font->height*12;
        SDL_Surface* surfaces[FC_LOAD_MAX_SURFACES];
        int num_surfaces = 1;
        surfaces[0] = FC_CreateSurface32(w, h);
        font->last_glyph.rect.x = FC_CACHE_PADDING;
        font->last_glyph.rect.y = FC_CACHE_PADDING;
        font->last_glyph.rect.w = 0;
        font->last_glyph.rect.h = font->height;

        source_string = font->loading_string;
        for(; *source_string != '\0'; source_string = U8_next(source_string))
        {
            memset(buff, 0, 5);
            if(!U8_charcpy(buff, source_string, 5))
                continue;
            glyph_surf = TTF_RenderText_Blended(ttf, buff, 0, white);
            if(glyph_surf == NULL)
                continue;

            // Try packing.  If it fails, create a new surface for the next cache level.
            packed = (FC_PackGlyphData(font, FC_GetCodepointFromUTF8(&buff_ptr, 0), (Uint16)glyph_surf->w, (Uint16)surfaces[num_surfaces-1]->w, (Uint16)surfaces[num_surfaces-1]->h) != NULL);
            if(!packed)
            {
                int i = num_surfaces-1;
                if(num_surfaces >= FC_LOAD_MAX_SURFACES)
                {
                    // Can't do any more!
                    FC_Log("SDL_FontCache error: Could not create enough cache surfaces to fit all of the loading string!\n");
                    SDL_DestroySurface(glyph_surf);
                    break;
                }

                // Upload the current surface to the glyph cache now so we can keep the cache level packing cursor up to date as we go.
                FC_UploadGlyphCache(font, i, surfaces[i]);
                SDL_DestroySurface(surfaces[i]);
                SDL_SetTextureBlendMode(font->glyph_cache[i], SDL_BLENDMODE_BLEND);
                // Update the glyph cursor to the new cache level.  We need to do this here because the actual cache lags behind our use of the packing above.
                font->last_glyph.cache_level = num_surfaces;


                surfaces[num_surfaces] = FC_CreateSurface32(w, h);
                num_surfaces++;
            }

            // Try packing for the new surface, then blit onto it.
            if(packed || FC_PackGlyphData(font, FC_GetCodepointFromUTF8(&buff_ptr, 0), (Uint16)glyph_surf->w, (Uint16)surfaces[num_surfaces-1]->w, (Uint16)surfaces[num_surfaces-1]->h) != NULL)
            {
                SDL_SetSurfaceBlendMode(glyph_surf, SDL_BLENDMODE_NONE);
                SDL_Rect srcRect = {0, 0, glyph_surf->w, glyph_surf->h};
                SDL_Rect destrect;
                destrect.x = (int)font->last_glyph.rect.x;
                destrect.y = (int)font->last_glyph.rect.y;
                destrect.w = (int)font->last_glyph.rect.w;
                destrect.h = (int)font->last_glyph.rect.h;
                SDL_BlitSurface(glyph_surf, &srcRect, surfaces[num_surfaces-1], &destrect);
            }

            SDL_DestroySurface(glyph_surf);
        }

        {
            int i = num_surfaces-1;
            FC_UploadGlyphCache(font, i, surfaces[i]);
            SDL_DestroySurface(surfaces[i]);
            SDL_SetTextureBlendMode(font->glyph_cache[i], SDL_BLENDMODE_BLEND);
        }
    }

    return 1;
}

Uint8 FC_LoadFont(FC_Font* font, FC_Target* renderer, const char* filename_ttf, Uint32 pointSize, SDL_Color color, unsigned int style)
{
    SDL_IOStream* iostream;

    if(font == NULL)
        return 0;

    iostream = SDL_IOFromFile(filename_ttf, "rb");

    if(iostream == NULL)
    {
        FC_Log("Unable to open file for reading: %s \n", SDL_GetError());
        return 0;
    }

    return FC_LoadFont_RW(font, renderer, iostream, 1, pointSize, color, style);
}

Uint8 FC_LoadFont_RW(FC_Font* font, FC_Target* renderer, SDL_IOStream* file_iostream_ttf, Uint8 own_rwops, Uint32 pointSize, SDL_Color color, unsigned int style)
{
    Uint8 result;
    TTF_Font* ttf;
    Uint8 outline;

    if(font == NULL)
        return 0;

    if(!TTF_WasInit() && TTF_Init() == false)
    {
        FC_Log("Unable to initialize SDL_ttf: %s \n", SDL_GetError());
        if(own_rwops)
            SDL_CloseIO(file_iostream_ttf);
        return 0;
    }

    ttf = TTF_OpenFontIO(file_iostream_ttf, own_rwops, (float)pointSize);

    if(ttf == NULL)
    {
        FC_Log("Unable to load TrueType font: %s \n", SDL_GetError());
        if(own_rwops)
            SDL_CloseIO(file_iostream_ttf);
        return 0;
    }

    TTF_SetFontHinting(ttf,TTF_HINTING_LIGHT); //hopefully helps font rendering on low-DPI displays

    outline = (style & TTF_STYLE_OUTLINE);
    if(outline)
    {
        style = style & ~TTF_STYLE_OUTLINE;
        TTF_SetFontOutline(ttf, 1);
    }
    TTF_SetFontStyle(ttf, style);

    result = FC_LoadFontFromTTF(font, renderer, ttf, color);

    // Can only load new (uncached) glyphs if we can keep the SDL_RWops open.
    font->owns_ttf_source = own_rwops;
    if(!own_rwops)
    {
        TTF_CloseFont(font->ttf_source);
        font->ttf_source = NULL;
    }

    //FC_Log("Loaded font with result: %u\n",result);

    return result;
}

void FC_ResetFontFromRendererReset(FC_Font* font, SDL_Renderer* renderer, Uint32 evType)
{
    TTF_Font* ttf;
    SDL_Color col;
    Uint8 owns_ttf;
    if (font == NULL)
        return;

    // Destroy glyph cache
    if (evType == SDL_EVENT_RENDER_TARGETS_RESET) {
        int i;
        for (i = 0; i < font->glyph_cache_count; ++i)
            SDL_DestroyTexture(font->glyph_cache[i]);
    }
    free(font->glyph_cache);

    ttf = font->ttf_source;
    col = font->default_color;
    owns_ttf = font->owns_ttf_source;
    FC_Init(font);

    // Can only reload glyphs if we own the SDL_RWops.
    if (owns_ttf)
        FC_LoadFontFromTTF(font, renderer, ttf, col);
    font->owns_ttf_source = owns_ttf;
}

void FC_ClearFont(FC_Font* font)
{
    int i;
    if(font == NULL)
        return;

    // Release resources
    if(font->owns_ttf_source)
        TTF_CloseFont(font->ttf_source);

    font->owns_ttf_source = 0;
    font->ttf_source = NULL;

    // Delete glyph map
    FC_MapFree(font->glyphs);
    font->glyphs = NULL;

    // Delete glyph cache
    for(i = 0; i < font->glyph_cache_count; ++i)
    {
        SDL_DestroyTexture(font->glyph_cache[i]);
    }
    free(font->glyph_cache);
    font->glyph_cache = NULL;

    // Reset font
    FC_Init(font);
}


void FC_FreeFont(FC_Font* font)
{
    int i;
    if(font == NULL)
        return;

    // Release resources
    if(font->owns_ttf_source)
        TTF_CloseFont(font->ttf_source);

    // Delete glyph map
    FC_MapFree(font->glyphs);

    // Delete glyph cache
    for(i = 0; i < font->glyph_cache_count; ++i)
    {
        SDL_DestroyTexture(font->glyph_cache[i]);
    }
    free(font->glyph_cache);

    free(font->loading_string);

    free(font);

    // If the last font has been freed; assume shutdown and free the global variables
    if (--NUM_EXISTING_FONTS <= 0)
    {
        free(ASCII_STRING);
        ASCII_STRING = NULL;

        free(fc_buffer);
        fc_buffer = NULL;
    }
}

int FC_GetNumCacheLevels(FC_Font* font)
{
    return font->glyph_cache_count;
}

Uint8 FC_AddGlyphToCache(FC_Font* font, SDL_Surface* glyph_surface)
{
    if(font == NULL || glyph_surface == NULL)
        return 0;

    SDL_SetSurfaceBlendMode(glyph_surface, SDL_BLENDMODE_NONE);
    FC_Image* dest = FC_GetGlyphCacheLevel(font, font->last_glyph.cache_level);
    if(dest == NULL)
        return 0;

    SDL_Renderer* renderer = font->renderer;
    SDL_Texture* img;
    SDL_FRect destrect;
    SDL_Texture* prev_target = SDL_GetRenderTarget(renderer);
    SDL_FRect prev_clip;
    SDL_Rect prev_viewport;
    int prev_logicalw = 0, prev_logicalh = 0;
    Uint8 prev_clip_enabled;
    float prev_scalex, prev_scaley;
    // only backup if previous target existed (SDL will preserve them for the default target)
    if (prev_target) {
        prev_clip_enabled = has_clip(renderer);
        if (prev_clip_enabled)
            prev_clip = get_clip(renderer);
        SDL_GetRenderViewport(renderer, &prev_viewport);
        SDL_GetRenderScale(renderer, &prev_scalex, &prev_scaley);
        SDL_GetRenderLogicalPresentation(renderer, &prev_logicalw, &prev_logicalh, NULL);
    }

    img = SDL_CreateTextureFromSurface(renderer, glyph_surface);

    destrect.x = (float)font->last_glyph.rect.x;
    destrect.y = (float)font->last_glyph.rect.y;
    destrect.w = (float)font->last_glyph.rect.w;
    destrect.h = (float)font->last_glyph.rect.h;
    SDL_SetRenderTarget(renderer, dest);
    SDL_RenderTextureRotated(renderer, img, NULL, &destrect, 0, NULL, SDL_FLIP_NONE);

    SDL_SetRenderTarget(renderer, prev_target);
    if (prev_target) {
        if (prev_clip_enabled)
            set_clip(renderer, &prev_clip);
        if (prev_logicalw && prev_logicalh)
            SDL_SetRenderLogicalPresentation(renderer, prev_logicalw, prev_logicalh,
                SDL_LOGICAL_PRESENTATION_DISABLED);
        else {
            SDL_SetRenderViewport(renderer, &prev_viewport);
            SDL_SetRenderScale(renderer, prev_scalex, prev_scaley);
        }
    }

    SDL_DestroyTexture(img);

    return 1;
}


unsigned int FC_GetNumCodepoints(FC_Font* font)
{
    FC_Map* glyphs;
    int i;
    unsigned int result = 0;
    if(font == NULL || font->glyphs == NULL)
        return 0;

    glyphs = font->glyphs;

    for(i = 0; i < glyphs->num_buckets; ++i)
    {
        FC_MapNode* node;
        for(node = glyphs->buckets[i]; node != NULL; node = node->next)
        {
            result++;
        }
    }

    return result;
}

void FC_GetCodepoints(FC_Font* font, Uint32* result)
{
    FC_Map* glyphs;
    int i;
    unsigned int count = 0;
    if(font == NULL || font->glyphs == NULL)
        return;

    glyphs = font->glyphs;

    for(i = 0; i < glyphs->num_buckets; ++i)
    {
        FC_MapNode* node;
        for(node = glyphs->buckets[i]; node != NULL; node = node->next)
        {
            result[count] = node->key;
            count++;
        }
    }
}

Uint8 FC_GetGlyphData(FC_Font* font, FC_GlyphData* result, Uint32 codepoint)
{
    FC_GlyphData* e = FC_MapFind(font->glyphs, codepoint);
    if(e == NULL)
    {
        char buff[5];
        float w, h;
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surf;
        FC_Image* cache_image;

        if(font->ttf_source == NULL)
            return 0;

        FC_GetUTF8FromCodepoint(buff, codepoint);
        //FC_Log("buff: %s\n",buff);

        cache_image = FC_GetGlyphCacheLevel(font, font->last_glyph.cache_level);
        if(cache_image == NULL)
        {
            FC_Log("SDL_FontCache: Failed to load cache image, so cannot add new glyphs!\n");
            return 0;
        }

        SDL_GetTextureSize(cache_image, &w, &h);

        surf = TTF_RenderText_Blended(font->ttf_source, buff, 0, white);
        if(surf == NULL)
        {
            return 0;
        }

        e = FC_PackGlyphData(font, codepoint, (Uint16)surf->w, (Uint16)w, (Uint16)h);
        if(e == NULL)
        {
            // Grow the cache
            FC_GrowGlyphCache(font);

            // Try packing again
            e = FC_PackGlyphData(font, codepoint, (Uint16)surf->w, (Uint16)w, (Uint16)h);
            if(e == NULL)
            {
                FC_Log("FC_GetGlyphData - packing failed.\n");
                SDL_DestroySurface(surf);
                return 0;
            }
        }

        // Render onto the cache texture
        FC_AddGlyphToCache(font, surf);

        SDL_DestroySurface(surf);
    }

    if(result != NULL && e != NULL)
        *result = *e;

    return 1;
}


FC_GlyphData* FC_SetGlyphData(FC_Font* font, Uint32 codepoint, FC_GlyphData glyph_data)
{
    return FC_MapInsert(font->glyphs, codepoint, glyph_data);
}



// Drawing
static FC_Rect FC_RenderLeft(FC_Font* font, FC_Target* dest, float x, float y, FC_Scale scale, const char* text)
{

    //FC_Log("Rendering text: %s\n",text);

    const char* c = text;
    FC_Rect srcRect;
    FC_Rect dstRect;
    FC_Rect dirtyRect = FC_MakeRect(x, y, 0, 0);

    FC_GlyphData glyph;
    Uint32 codepoint;

    float destX = x;
    float destY = y;
    float destH;
    float destLineSpacing;
    float destLetterSpacing;

    if(font == NULL)
        return dirtyRect;

    destH = font->height * scale.y;
    destLineSpacing = (float)(font->lineSpacing)*scale.y;
    destLetterSpacing = (float)(font->letterSpacing)*scale.x;

    if(c == NULL || font->glyph_cache_count == 0 || dest == NULL)
        return dirtyRect;

    float newlineX = x;

    for(; *c != '\0'; c++)
    {
        if(*c == '\n')
        {
            destX = newlineX;
            destY += destH + destLineSpacing;
            continue;
        }

        codepoint = FC_GetCodepointFromUTF8(&c, 1);  // Increments 'c' to skip the extra UTF-8 bytes
        //FC_Log("codept: %u\n",codepoint);
        if(!FC_GetGlyphData(font, &glyph, codepoint))
        {
            //FC_Log("Couldn't get glyph data for codepoint: %u\n",codepoint);
            codepoint = ' ';
            if(!FC_GetGlyphData(font, &glyph, codepoint))
                continue;  // Skip bad characters
        }

        if (codepoint == ' ')
        {
            destX += glyph.rect.w*scale.x + destLetterSpacing;
            continue;
        }
        /*if(destX >= dest->w)
            continue;
        if(destY >= dest->h)
            continue;*/

        srcRect = glyph.rect;
        dstRect = fc_render_callback(FC_GetGlyphCacheLevel(font, glyph.cache_level), &srcRect, dest, destX, destY, scale.x, scale.y);
        if(dirtyRect.w == 0 || dirtyRect.h == 0)
            dirtyRect = dstRect;
        else
            dirtyRect = FC_RectUnion(dirtyRect, dstRect);

        destX += glyph.rect.w*scale.x + destLetterSpacing;
    }

    return dirtyRect;
}

static void set_color_for_all_caches(FC_Font* font, SDL_Color color)
{
    // TODO: How can I predict which glyph caches are to be used?
    FC_Image* img;
    int i;
    int num_levels = FC_GetNumCacheLevels(font);
    for(i = 0; i < num_levels; ++i)
    {
        img = FC_GetGlyphCacheLevel(font, i);
        set_color(img, color.r, color.g, color.b, FC_GET_ALPHA(color));
    }
}

FC_Rect FC_Draw(FC_Font* font, FC_Target* dest, float x, float y, const char* formatted_text, ...)
{
    if(formatted_text == NULL || font == NULL)
        return FC_MakeRect(x, y, 0, 0);

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    set_color_for_all_caches(font, font->default_color);

    return FC_RenderLeft(font, dest, x, y, FC_MakeScale(1,1), fc_buffer);
}



typedef struct FC_StringList
{
    char* value;
    struct FC_StringList* next;
} FC_StringList;

void FC_StringListFree(FC_StringList* node)
{
    // Delete the nodes in order
    while(node != NULL)
    {
        FC_StringList* last = node;
        node = node->next;

        free(last->value);
        free(last);
    }
}

FC_StringList** FC_StringListPushBack(FC_StringList** node, char* value, Uint8 copy)
{
    if(node == NULL)
    {
        return NULL;
    }

    // Get to the last node
    while(*node != NULL)
    {
        node = &(*node)->next;
    }

    *node = (FC_StringList*)malloc(sizeof(FC_StringList));

    (*node)->value = (copy? U8_strdup(value) : value);
    (*node)->next = NULL;

    return node;
}

FC_StringList** FC_StringListPushBackBytes(FC_StringList** node, const char* data, int num_bytes)
{
    if(node == NULL)
    {
        return node;
    }

    // Get to the last node
    while(*node != NULL)
    {
        node = &(*node)->next;
    }

    *node = (FC_StringList*)malloc(sizeof(FC_StringList));

    (*node)->value = (char*)malloc((size_t)(num_bytes + 1));
    memcpy((*node)->value, data, (size_t)num_bytes);
    (*node)->value[num_bytes] = '\0';
    (*node)->next = NULL;

    return node;
}

static FC_StringList* FC_Explode(const char* text, char delimiter)
{
    FC_StringList* head;
    FC_StringList* new_node;
    FC_StringList** node;
    const char* start;
    const char* end;
    unsigned int size;
    if(text == NULL)
        return NULL;

    head = NULL;
    node = &head;

    // Doesn't technically support UTF-8, but it's probably fine, right?
    size = 0;
    start = end = text;
    while(1)
    {
        if(*end == delimiter || *end == '\0')
        {
            *node = (FC_StringList*)malloc(sizeof(FC_StringList));
            new_node = *node;

            new_node->value = (char*)malloc(size + 1);
            memcpy(new_node->value, start, size);
            new_node->value[size] = '\0';

            new_node->next = NULL;

            if(*end == '\0')
                break;

            node = &((*node)->next);
            start = end+1;
            size = 0;
        }
        else
            ++size;

        ++end;
    }

    return head;
}

static FC_StringList* FC_ExplodeBreakingSpace(const char* text, FC_StringList** spaces)
{
    FC_StringList* head;
    FC_StringList** node;
    const char* start;
    const char* end;
    unsigned int size;
    if(text == NULL)
        return NULL;

    head = NULL;
    node = &head;

    // Warning: spaces must not be initialized before this function
    *spaces = NULL;

    // Doesn't technically support UTF-8, but it's probably fine, right?
    size = 0;
    start = end = text;
    while(1)
    {
        // Add any characters here that should make separate words (except for \n?)
        if(*end == ' ' || *end == ')' || *end == '\t' || *end == '\0')
        {
            FC_StringListPushBackBytes(node, start, (int)size);
            FC_StringListPushBackBytes(spaces, end, 1);

            if(*end == '\0')
                break;

            node = &((*node)->next);
            start = end+1;
            size = 0;
        }
        else
            ++size;

        ++end;
    }

    return head;
}

static FC_StringList* FC_ExplodeAndKeep(const char* text, char delimiter)
{
    FC_StringList* head;
    FC_StringList** node;
    const char* start;
    const char* end;
    unsigned int size;
    if(text == NULL)
        return NULL;

    head = NULL;
    node = &head;

    // Doesn't technically support UTF-8, but it's probably fine, right?
    size = 0;
    start = end = text;
    while(1)
    {
        if(*end == delimiter || *end == '\0')
        {
            FC_StringListPushBackBytes(node, start, (int)size);

            if(*end == '\0')
                break;

            node = &((*node)->next);
            start = end;
            size = 1;
        }
        else
            ++size;

        ++end;
    }

    return head;
}

static void FC_RenderAlign(FC_Font* font, FC_Target* dest, float x, float y, float width, FC_Scale scale, FC_AlignEnum align, const char* text)
{
    switch(align)
    {
        case FC_ALIGN_LEFT:
            FC_RenderLeft(font, dest, x, y, scale, text);
            break;
        case FC_ALIGN_CENTER:
            FC_RenderCenter(font, dest, x + width/2, y, scale, text);
            break;
        case FC_ALIGN_RIGHT:
            FC_RenderRight(font, dest, x + width, y, scale, text);
            break;
    }
}

static FC_StringList* FC_GetBufferFitToColumn(FC_Font* font, int width, Uint8 keep_newlines)
{
    FC_StringList* result = NULL;
    FC_StringList** current = &result;

    FC_StringList *ls, *iter;

    ls = (keep_newlines? FC_ExplodeAndKeep(fc_buffer, '\n') : FC_Explode(fc_buffer, '\n'));
    for(iter = ls; iter != NULL; iter = iter->next)
    {
        char* line = iter->value;

        // If line is too long, then add words one at a time until we go over.
        if(width > 0 && FC_GetWidth(font, "%s", line) > width)
        {
            FC_StringList *words, *word_iter, *spaces=NULL, *spaces_iter;

            words = FC_ExplodeBreakingSpace(line, &spaces);
            // Skip the first word for the iterator, so there will always be at least one word per line
            if(words != NULL){
                line = new_concat(words->value, spaces->value);
                for(word_iter = words->next, spaces_iter = spaces->next; word_iter != NULL && spaces_iter != NULL; word_iter = word_iter->next, spaces_iter = spaces_iter->next)
                {
                    char* line_plus_word = new_concat(line, word_iter->value);
                    char* word_plus_space = new_concat(word_iter->value, spaces_iter->value);
                    if(FC_GetWidth(font, "%s", line_plus_word) > width)
                    {
                        current = FC_StringListPushBack(current, line, 0);

                        line = word_plus_space;
                    }
                    else
                    {
                        replace_concat(&line, word_plus_space);
                        free(word_plus_space);
                    }
                    free(line_plus_word);
                }
            }
            current = FC_StringListPushBack(current, line, 0);
            FC_StringListFree(words);
            FC_StringListFree(spaces);
        }
        else
        {
            current = FC_StringListPushBack(current, line, 0);
            iter->value = NULL;
        }
    }
    FC_StringListFree(ls);

    return result;
}

static void FC_DrawColumnFromBuffer(FC_Font* font, FC_Target* dest, FC_Rect box, int* total_height, FC_Scale scale, FC_AlignEnum align)
{
    int y = (int)box.y;
    FC_StringList *ls, *iter;

    ls = FC_GetBufferFitToColumn(font, (int)(box.w), 0);
    for(iter = ls; iter != NULL; iter = iter->next)
    {
        FC_RenderAlign(font, dest, box.x, (float)y, box.w, scale, align, iter->value);
        y += (int)(FC_GetLineHeight(font) * scale.y);
    }
    FC_StringListFree(ls);

    if(total_height != NULL)
        *total_height = y - (int)box.y;
}

FC_Rect FC_DrawColumn(FC_Font* font, FC_Target* dest, float x, float y, Uint16 width, const char* formatted_text, ...)
{
    FC_Rect box = {x, y, width, 0};
    int total_height;

    if(formatted_text == NULL || font == NULL)
        return FC_MakeRect(x, y, 0, 0);

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    set_color_for_all_caches(font, font->default_color);

    FC_DrawColumnFromBuffer(font, dest, box, &total_height, FC_MakeScale(1,1), FC_ALIGN_LEFT);

    return FC_MakeRect(box.x, box.y, width, (float)total_height);
}

FC_Rect FC_DrawColumnAlign(FC_Font* font, FC_Target* dest, float x, float y, Uint16 width, FC_AlignEnum align, const char* formatted_text, ...)
{
    FC_Rect box = {x, y, width, 0};
    int total_height;

    if(formatted_text == NULL || font == NULL)
        return FC_MakeRect(x, y, 0, 0);

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    set_color_for_all_caches(font, font->default_color);

    switch(align)
    {
    case FC_ALIGN_CENTER:
        box.x -= width/2;
        break;
    case FC_ALIGN_RIGHT:
        box.x -= width;
        break;
    default:
        break;
    }

    FC_DrawColumnFromBuffer(font, dest, box, &total_height, FC_MakeScale(1,1), align);

    return FC_MakeRect(box.x, box.y, width, (float)total_height);
}

FC_Rect FC_DrawColumnScale(FC_Font* font, FC_Target* dest, float x, float y, Uint16 width, FC_Scale scale, const char* formatted_text, ...)
{
    FC_Rect box = {x, y, width, 0};
    int total_height;

    if(formatted_text == NULL || font == NULL)
        return FC_MakeRect(x, y, 0, 0);

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    set_color_for_all_caches(font, font->default_color);

    FC_DrawColumnFromBuffer(font, dest, box, &total_height, scale, FC_ALIGN_LEFT);

    return FC_MakeRect((float)box.x, (float)box.y, width, (float)total_height);
}

FC_Rect FC_DrawColumnColor(FC_Font* font, FC_Target* dest, float x, float y, Uint16 width, SDL_Color color, const char* formatted_text, ...)
{
    FC_Rect box = {x, y, (float)width, 0.0f};
    int total_height = 0;

    float drawWidth = FC_GetWidth(font,formatted_text);
    if(drawWidth > width){
        drawWidth = (float)width;
    }

    if(formatted_text == NULL || font == NULL)
        return FC_MakeRect(x, y, 0, 0);

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    set_color_for_all_caches(font, color);

    FC_DrawColumnFromBuffer(font, dest, box, &total_height, FC_MakeScale(1,1), FC_ALIGN_LEFT);

    return FC_MakeRect((float)box.x, (float)box.y, drawWidth, (float)total_height);
}

FC_Rect FC_DrawColumnColorAlign(FC_Font* font, FC_Target* dest, float x, float y, Uint16 width, SDL_Color color, FC_AlignEnum alignment, const char* formatted_text, ...)
{
    FC_Rect box = {x, y, (float)width, 0.0f};
    int total_height = 0;

    float drawWidth = FC_GetWidth(font,formatted_text);
    if(drawWidth > width){
        drawWidth = (float)width;
    }

    if(formatted_text == NULL || font == NULL)
        return FC_MakeRect(x, y, 0, 0);

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    set_color_for_all_caches(font, color);

    FC_DrawColumnFromBuffer(font, dest, box, &total_height, FC_MakeScale(1,1), alignment);

    return FC_MakeRect((float)box.x, (float)box.y, drawWidth, (float)total_height);
}

FC_Rect FC_DrawColumnColorScale(FC_Font* font, FC_Target* dest, float x, float y, Uint16 width, SDL_Color color, const float scale, const char* formatted_text, ...)
{   
    int total_height = 0;
    float reqWidth = ((float)width)*scale;
    FC_Rect box = {x, y, (float)(width/scale), 0.0f};

    float drawWidth = FC_GetWidth(font,formatted_text)*scale;
    if(drawWidth > reqWidth){
        drawWidth = reqWidth;
    }

    if(formatted_text == NULL || font == NULL)
        return FC_MakeRect(x, y, 0.0f, 0.0f);

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    set_color_for_all_caches(font, color);

    FC_DrawColumnFromBuffer(font, dest, box, &total_height, FC_MakeScale(scale,scale), FC_ALIGN_LEFT);

    return FC_MakeRect((float)box.x, (float)box.y, drawWidth, (float)total_height);
}

FC_Rect FC_DrawColumnEffect(FC_Font* font, FC_Target* dest, float x, float y, Uint16 width, FC_Effect effect, const char* formatted_text, ...)
{
    FC_Rect box = {x, y, (float)width, 0.0f};
    int total_height;

    if(formatted_text == NULL || font == NULL)
        return FC_MakeRect(x, y, 0, 0);

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    set_color_for_all_caches(font, effect.color);

    switch(effect.alignment)
    {
    case FC_ALIGN_CENTER:
        box.x -= width/2;
        break;
    case FC_ALIGN_RIGHT:
        box.x -= width;
        break;
    default:
        break;
    }

    FC_DrawColumnFromBuffer(font, dest, box, &total_height, effect.scale, effect.alignment);

    return FC_MakeRect((float)box.x, (float)box.y, width, (float)total_height);
}

static FC_Rect FC_RenderCenter(FC_Font* font, FC_Target* dest, float x, float y, FC_Scale scale, const char* text)
{
    FC_Rect result = {x, y, 0, 0};
    if(text == NULL || font == NULL)
        return result;

    char* str = U8_strdup(text);
    char* del = str;
    char* c;

    // Go through str, when you find a \n, replace it with \0 and print it
    // then move down, back, and continue.
    for(c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            result = FC_RectUnion(FC_RenderLeft(font, dest, x - scale.x*FC_GetWidth(font, "%s", str)/2.0f, y, scale, str), result);
            *c = '\n';
            c++;
            str = c;
            y += scale.y*font->height;
        }
        else
            c++;
    }

    result = FC_RectUnion(FC_RenderLeft(font, dest, x - scale.x*FC_GetWidth(font, "%s", str)/2.0f, y, scale, str), result);

    free(del);
    return result;
}

static FC_Rect FC_RenderRight(FC_Font* font, FC_Target* dest, float x, float y, FC_Scale scale, const char* text)
{
    FC_Rect result = {x, y, 0, 0};
    if(text == NULL || font == NULL)
        return result;

    char* str = U8_strdup(text);
    char* del = str;
    char* c;

    for(c = str; *c != '\0';)
    {
        if(*c == '\n')
        {
            *c = '\0';
            result = FC_RectUnion(FC_RenderLeft(font, dest, x - scale.x*FC_GetWidth(font, "%s", str), y, scale, str), result);
            *c = '\n';
            c++;
            str = c;
            y += scale.y*font->height;
        }
        else
            c++;
    }

    result = FC_RectUnion(FC_RenderLeft(font, dest, x - scale.x*FC_GetWidth(font, "%s", str), y, scale, str), result);

    free(del);
    return result;
}



FC_Rect FC_DrawScale(FC_Font* font, FC_Target* dest, float x, float y, FC_Scale scale, const char* formatted_text, ...)
{
    if(formatted_text == NULL || font == NULL)
        return FC_MakeRect(x, y, 0, 0);

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    set_color_for_all_caches(font, font->default_color);

    return FC_RenderLeft(font, dest, x, y, scale, fc_buffer);
}

FC_Rect FC_DrawAlign(FC_Font* font, FC_Target* dest, float x, float y, FC_AlignEnum align, const char* formatted_text, ...)
{
    if(formatted_text == NULL || font == NULL)
        return FC_MakeRect(x, y, 0, 0);

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    set_color_for_all_caches(font, font->default_color);

    FC_Rect result;
    switch(align)
    {
        case FC_ALIGN_LEFT:
            result = FC_RenderLeft(font, dest, x, y, FC_MakeScale(1,1), fc_buffer);
            break;
        case FC_ALIGN_CENTER:
            result = FC_RenderCenter(font, dest, x, y, FC_MakeScale(1,1), fc_buffer);
            break;
        case FC_ALIGN_RIGHT:
            result = FC_RenderRight(font, dest, x, y, FC_MakeScale(1,1), fc_buffer);
            break;
        default:
            result = FC_MakeRect(x, y, 0, 0);
            break;
    }

    return result;
}

FC_Rect FC_DrawColor(FC_Font* font, FC_Target* dest, float x, float y, SDL_Color color, const char* formatted_text, ...)
{
    if(formatted_text == NULL || font == NULL)
        return FC_MakeRect(x, y, 0, 0);

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    set_color_for_all_caches(font, color);

    return FC_RenderLeft(font, dest, x, y, FC_MakeScale(1,1), fc_buffer);
}


FC_Rect FC_DrawEffect(FC_Font* font, FC_Target* dest, float x, float y, FC_Effect effect, const char* formatted_text, ...)
{
    if(formatted_text == NULL || font == NULL)
        return FC_MakeRect(x, y, 0, 0);

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    set_color_for_all_caches(font, effect.color);

    FC_Rect result;
    switch(effect.alignment)
    {
        case FC_ALIGN_LEFT:
            result = FC_RenderLeft(font, dest, x, y, effect.scale, fc_buffer);
            break;
        case FC_ALIGN_CENTER:
            result = FC_RenderCenter(font, dest, x, y, effect.scale, fc_buffer);
            break;
        case FC_ALIGN_RIGHT:
            result = FC_RenderRight(font, dest, x, y, effect.scale, fc_buffer);
            break;
        default:
            result = FC_MakeRect(x, y, 0, 0);
            break;
    }

    return result;
}




// Getters


FC_FilterEnum FC_GetFilterMode(FC_Font* font)
{
    if(font == NULL)
        return FC_FILTER_NEAREST;

    return font->filter;
}

Uint16 FC_GetLineHeight(FC_Font* font)
{
    if(font == NULL)
        return 0;

    return font->height;
}

Uint16 FC_GetHeight(FC_Font* font, const char* formatted_text, ...)
{
    if(formatted_text == NULL || font == NULL)
        return 0;

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    Uint16 numLines = 1;
    const char* c;

    for (c = fc_buffer; *c != '\0'; c++)
    {
        if(*c == '\n')
            numLines++;
    }

    //   Actual height of letter region + line spacing
    return (Uint16)(font->height*numLines + font->lineSpacing*(numLines - 1));  //height*numLines;
}

Uint16 FC_GetWidth(FC_Font* font, const char* formatted_text, ...)
{
    if(formatted_text == NULL || font == NULL)
        return 0;

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    const char* c;
    Uint16 width = 0;
    Uint16 bigWidth = 0;  // Allows for multi-line strings

    for (c = fc_buffer; *c != '\0'; c++)
    {
        if(*c == '\n')
        {
            bigWidth = bigWidth >= width? bigWidth : width;
            width = 0;
            continue;
        }

        FC_GlyphData glyph;
        Uint32 codepoint = FC_GetCodepointFromUTF8(&c, 1);
        if(FC_GetGlyphData(font, &glyph, codepoint) || FC_GetGlyphData(font, &glyph, ' '))
            width = (Uint16)(width + glyph.rect.w);
    }
    bigWidth = bigWidth >= width? bigWidth : width;

    return bigWidth;
}

// If width == -1, use no width limit
FC_Rect FC_GetCharacterOffset(FC_Font* font, Uint16 position_index, int column_width, const char* formatted_text, ...)
{
    FC_Rect result = {0, 0, 1, FC_GetLineHeight(font)};
    FC_StringList *ls, *iter;
    int num_lines = 0;
    Uint8 done = 0;

    if(formatted_text == NULL || column_width == 0 || position_index == 0 || font == NULL)
        return result;

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    ls = FC_GetBufferFitToColumn(font, column_width, 1);
    for(iter = ls; iter != NULL;)
    {
        char* line;
        FC_StringList* next_iter = iter->next;

        ++num_lines;
        for(line = iter->value; line != NULL && *line != '\0'; line = (char*)U8_next(line))
        {
            --position_index;
            if(position_index == 0)
            {
                // FIXME: Doesn't handle box-wrapped newlines correctly
                line = (char*)U8_next(line);
                line[0] = '\0';
                result.x = FC_GetWidth(font, "%s", iter->value);
                done = 1;
                break;
            }
        }
        if(done)
            break;

        // Prevent line wrapping if there are no more lines
        if(next_iter == NULL && !done)
            result.x = FC_GetWidth(font, "%s", iter->value);
        iter = next_iter;
    }
    FC_StringListFree(ls);

    if(num_lines > 1)
    {
        result.y = (float)((num_lines - 1) * FC_GetLineHeight(font));
    }

    return result;
}


Uint16 FC_GetColumnHeight(FC_Font* font, Uint16 width, const char* formatted_text, ...)
{
    int y = 0;

    FC_StringList *ls, *iter;

    if(font == NULL)
        return 0;

    if(formatted_text == NULL || width == 0)
        return font->height;

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    ls = FC_GetBufferFitToColumn(font, width, 0);
    for(iter = ls; iter != NULL; iter = iter->next)
    {
        y += FC_GetLineHeight(font);
    }
    FC_StringListFree(ls);

    return (Uint16)y;
}

static float FC_GetAscentFromCodepoint(FC_Font* font, Uint32 codepoint)
{
    FC_GlyphData glyph;

    if(font == NULL)
        return 0;

    // FIXME: Store ascent so we can return it here
    FC_GetGlyphData(font, &glyph, codepoint);
    return glyph.rect.h;
}

static float FC_GetDescentFromCodepoint(FC_Font* font, Uint32 codepoint)
{
    FC_GlyphData glyph;

    if(font == NULL)
        return 0;

    // FIXME: Store descent so we can return it here
    FC_GetGlyphData(font, &glyph, codepoint);
    return glyph.rect.h;
}

float FC_GetAscent(FC_Font* font, const char* formatted_text, ...)
{
    Uint32 codepoint;
    float max, ascent;
    const char* c;

    if(font == NULL)
        return 0;

    if(formatted_text == NULL)
        return font->ascent;

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    max = 0;
    c = fc_buffer;

    while(*c != '\0')
    {
        codepoint = FC_GetCodepointFromUTF8(&c, 1);
        if(codepoint != 0)
        {
            ascent = FC_GetAscentFromCodepoint(font, codepoint);
            if(ascent > max)
                max = ascent;
        }
        ++c;
    }
    return max;
}

float FC_GetDescent(FC_Font* font, const char* formatted_text, ...)
{
    Uint32 codepoint;
    float max, descent;
    const char* c;

    if(font == NULL)
        return 0;

    if(formatted_text == NULL)
        return font->descent;

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    max = 0;
    c = fc_buffer;

    while(*c != '\0')
    {
        codepoint = FC_GetCodepointFromUTF8(&c, 1);
        if(codepoint != 0)
        {
            descent = FC_GetDescentFromCodepoint(font, codepoint);
            if(descent > max)
                max = descent;
        }
        ++c;
    }
    return max;
}

int FC_GetBaseline(FC_Font* font)
{
    if(font == NULL)
        return 0;

    return font->baseline;
}

int FC_GetSpacing(FC_Font* font)
{
    if(font == NULL)
        return 0;

    return font->letterSpacing;
}

int FC_GetLineSpacing(FC_Font* font)
{
    if(font == NULL)
        return 0;

    return font->lineSpacing;
}

Uint16 FC_GetMaxWidth(FC_Font* font)
{
    if(font == NULL)
        return 0;

    return font->maxWidth;
}

SDL_Color FC_GetDefaultColor(FC_Font* font)
{
    if(font == NULL)
    {
        SDL_Color c = {0,0,0,255};
        return c;
    }

    return font->default_color;
}

FC_Rect FC_GetBounds(FC_Font* font, float x, float y, FC_AlignEnum align, FC_Scale scale, const char* formatted_text, ...)
{
    FC_Rect result = {x, y, 0, 0};

    if(formatted_text == NULL)
        return result;

    // Create a temp buffer while GetWidth and GetHeight use fc_buffer.
    char* temp = (char*)malloc(fc_buffer_size);
    FC_EXTRACT_VARARGS(temp, formatted_text);

    result.w = (float)(FC_GetWidth(font, "%s", temp)) * scale.x;
    result.h = (float)(FC_GetHeight(font, "%s", temp)) * scale.y;

    switch(align)
    {
        case FC_ALIGN_LEFT:
            break;
        case FC_ALIGN_CENTER:
            result.x -= result.w/2;
            break;
        case FC_ALIGN_RIGHT:
            result.x -= result.w;
            break;
        default:
            break;
    }

    free(temp);

    return result;
}

Uint8 FC_InRect(float x, float y, FC_Rect input_rect)
{
    return (input_rect.x <= x && x <= input_rect.x + input_rect.w && input_rect.y <= y && y <= input_rect.y + input_rect.h);
}

// TODO: Make it work with alignment
Uint16 FC_GetPositionFromOffset(FC_Font* font, float x, float y, int column_width, const char* formatted_text, ...)
{
    FC_StringList *ls, *iter;
    Uint8 done = 0;
    int height = FC_GetLineHeight(font);
    Uint16 position = 0;
    int current_x = 0;
    int current_y = 0;
    FC_GlyphData glyph_data;

    if(formatted_text == NULL || column_width == 0 || font == NULL)
        return 0;

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    ls = FC_GetBufferFitToColumn(font, column_width, 1);
    for(iter = ls; iter != NULL; iter = iter->next)
    {
        char* line;

        for(line = iter->value; line != NULL && *line != '\0'; line = (char*)U8_next(line))
        {
            if(FC_GetGlyphData(font, &glyph_data, FC_GetCodepointFromUTF8((const char**)&line, 0)))
            {
                if(FC_InRect(x, y, FC_MakeRect((float)current_x, (float)current_y, (float)glyph_data.rect.w, (float)glyph_data.rect.h)))
                {
                    done = 1;
                    break;
                }

                current_x += (int)glyph_data.rect.w;
            }
            position++;
        }
        if(done)
            break;

        current_x = 0;
        current_y += height;
        if(y < (float)current_y)
            break;
    }
    FC_StringListFree(ls);

    return position;
}

int FC_GetWrappedText(FC_Font* font, char* result, int max_result_size, Uint16 width, const char* formatted_text, ...)
{
    FC_StringList *ls, *iter;

    if(font == NULL)
        return 0;

    if(formatted_text == NULL || width == 0)
        return 0;

    FC_EXTRACT_VARARGS(fc_buffer, formatted_text);

    ls = FC_GetBufferFitToColumn(font, width, 0);
    int size_so_far = 0;
    int size_remaining = max_result_size-1; // reserve for \0
    for(iter = ls; iter != NULL && size_remaining > 0; iter = iter->next)
    {
        // Copy as much of this line as we can
        int len = (int)strlen(iter->value);
        int num_bytes = FC_MIN(len, size_remaining);
        memcpy(&result[size_so_far], iter->value, (size_t)num_bytes);
        size_so_far += num_bytes;
        size_remaining -= num_bytes;

        // If there's another line, add newline character
        if(size_remaining > 0 && iter->next != NULL)
        {
            --size_remaining;
            result[size_so_far] = '\n';
            ++size_so_far;
        }
    }
    FC_StringListFree(ls);

    result[size_so_far] = '\0';

    return size_so_far;
}



// Setters


void FC_SetFilterMode(FC_Font* font, FC_FilterEnum filter)
{
    if(font == NULL)
        return;

    if(font->filter != filter)
    {
        font->filter = filter;
    }
}


void FC_SetSpacing(FC_Font* font, int LetterSpacing)
{
    if(font == NULL)
        return;

    font->letterSpacing = LetterSpacing;
}

void FC_SetLineSpacing(FC_Font* font, int LineSpacing)
{
    if(font == NULL)
        return;

    font->lineSpacing = LineSpacing;
}

void FC_SetDefaultColor(FC_Font* font, SDL_Color color)
{
    if(font == NULL)
        return;

    font->default_color = color;
}
