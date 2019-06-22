#include "preproc.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "strbuf.h"

static const char frag_shader_prologue[] =
    "uniform vec3 iResolution;\n"
    "uniform float iTime;\n"
    "uniform float iTimeDelta;\n"
    "uniform float iFrame;\n"
    "uniform float iChannelTime[4];\n"
    "uniform vec4 iMouse;\n"
    "uniform vec4 iDate;\n"
    "uniform float iSampleRate;\n"
    "uniform vec3 iChannelResolution[4];\n"
    ;

static const char frag_shader_epilogue[] =
    "void main() {\n"
    "    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
    "    mainImage(gl_FragColor, gl_FragCoord.xy);\n"
    "    gl_FragColor.a = 1.0;\n"
    "}\n"
    ;

static const char *pragma_type(const char *line)
{
    if (!*line)
        return NULL;
    while (isspace(*line))
        if (!*++line)
            return NULL;
    if (*line++ != '#')
        return NULL;
    while (isspace(*line))
        if (!*++line)
            return NULL;
    if (strncmp(line, "pragma", 6) != 0)
        return NULL;
    line += 6;
    if (!isspace(*line))
        return NULL;
    if (!*++line)
        return NULL;
    while (isspace(*line))
        if (!*++line)
            return NULL;
    
    return line;
}

// #include <stdio.h>
// static void test_prag(void)
// {
//     printf("testing\n");
//     assert(!pragma_type(""));
//     assert(!pragma_type("  #"));
//     assert(!pragma_type("#  "));
//     assert(!pragma_type("# p"));
//     assert(!pragma_type("#p"));
//     assert(!pragma_type("#pragma"));
//     assert(!pragma_type(" # pragma\t"));
//     assert(!pragma_type("#\tpragma   "));
//     assert(pragma_type("#pragma yes")[0] == 'y');
//     assert(pragma_type(" #pragma yes")[0] == 'y');
//     assert(pragma_type("\t#pragma yes")[0] == 'y');
//     assert(pragma_type("# pragma yes")[0] == 'y');
//     assert(pragma_type("# \tpragma yes")[0] == 'y');
//     assert(pragma_type(" # pragma yes")[0] == 'y');
//     assert(pragma_type("\t#\tpragma yes")[0] == 'y');
//     assert(pragma_type("#pragma    yes")[0] == 'y');
//     assert(pragma_type("#pragma  \t yes")[0] == 'y');
//     assert(pragma_type("#pragma   \tyes")[0] == 'y');
//     printf("test OK\n");
// }

static bool process_map(const char *p, shader *sh)
{
    // # pragma map {name}=image:{file}
    // # pragma map {name}=builtin:RGBA Noise Small
    // # pragma map {name}=builtin:RGBA Noise Medium
    // # pragma map {name}=builtin:Back Buffer
    // # pragma map {name}=perip_map4:{dev};{baud}?
    
}

static bool process_pragma(const char *p, shader *sh)
{
    const char *end;
    for (end = p; *end && !isspace(*end); end++)
        continue;
    ptrdiff_t len = end - p;
    if (len == 3 !strncmp(p, "map", 3))
        return process_map(end, sh);
    // else if (len == 3 && !strncmp(p, "use", 3))
    //     return process_use(end, sh);
    else {
        fprintf(stderr, "unknown pragma \"%s\"\n", p);
        return false;
    }
}

static strbuf collect_source(FILE *f, shader *sh)
{
    char line[1024];
    strbuf buf = create_strbuf();
    while (fgets(line, sizeof line, f)) {
        const char *p = pragma_type(line);
        if (p) {
            if (!process_pragma(p, sh)) {
                return NULL;
            }
        } else {
            strbuf_append(buf, line, -1);
        }
    }
    return buf;
}

shader *read_shader(FILE *f)
{
    shader *sh = calloc(1, *sh);
    sh->source = create_str_array();

    strbuf srcbuf = collect_source(f, sh);
    if (!srcbuf) {
        destroy_shader(sh);
        return NULL;
    }

    str_array_append(sh->source,
                     frag_shader_prologue,
                     sizeof frag_shader_prologue - 1);
    str_array_extend(sh->source, srcbuf);
    str_array_append(sh->source,
                     frag_shader_epilogue,
                     sizeof frag_shader_epilogue - 1);

    return NULL;

}

void destroy_shader(shader *sh)
{
    destroy_str_array(sh->source);
    for (size_t i = 0; i < count; i++) {
        free(sh->attachments[i].name);
        free(sh->attachments[i].desc);
    }
    free(sh->attachments);
    free(sh);
}
