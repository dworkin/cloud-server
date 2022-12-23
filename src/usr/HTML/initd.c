# include "markdown.h"

void create()
{
    compile_object("sys/markdown");
    compile_object("sys/line");
    compile_object("sys/encode");
    compile_object(OBJECT_PATH(MarkdownPhrase));
    compile_object(OBJECT_PATH(MarkdownCode));
    compile_object(OBJECT_PATH(MarkdownLink));
    compile_object(OBJECT_PATH(MarkdownLine));
    compile_object(OBJECT_PATH(MarkdownList));
    compile_object(OBJECT_PATH(MarkdownParagraph));
}
