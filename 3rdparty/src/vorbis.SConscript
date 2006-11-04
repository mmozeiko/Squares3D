#
Import("env lib suffix")

headers = Split("""masking.h mdct.h misc.h lsp.h backends.h
                   bitrate.h codebook.h codec_internal.h
                   registry.h envelope.h highlevel.h lookup.h
                   lookup_data.h lpc.h os.h psy.h scales.h
                   smallft.h window.h""")

sources = Split("""analysis.c mdct.c bitrate.c block.c lsp.c
                   codebook.c envelope.c floor0.c floor1.c
                   info.c lookup.c lpc.c mapping0.c psy.c
                   registry.c res0.c sharedbook.c smallft.c
                   synthesis.c window.c""")

env.StaticLibrary(lib + "/vorbis" + suffix, ["vorbis/" + x for x in sources])
