#include "../load_bin.inc"
#include <Zydis/Zydis.h>

int main()
{
#ifndef DISAS_BENCH_NO_FORMAT
    ZydisFormatter formatter;
    if (!ZYAN_SUCCESS(ZydisFormatterInit(
        &formatter,
        ZYDIS_FORMATTER_STYLE_INTEL
    )))
    {
        fputs("Unable to initialize instruction formatter\n", stderr);
        return 1;
    }
#endif

    uint8_t *xul_code = NULL;
    size_t xul_code_len = 0;
    if (!read_xul_dll(&xul_code, &xul_code_len))
    {
        fputs("Can't read xul.dll\n", stderr);
        return 1;
    }

    ZydisDecoder decoder;
    ZydisDecoderInit(
        &decoder,
        ZYDIS_MACHINE_MODE_LONG_64,
        ZYDIS_ADDRESS_WIDTH_64
    );

#ifdef DISAS_BENCH_DECODE_MINIMAL
    ZydisDecoderEnableMode(
        &decoder,
        ZYDIS_DECODER_MODE_MINIMAL,
        ZYAN_TRUE
    );
#endif

    size_t num_valid_insns = 0;
    size_t num_bad_insn = 0;
    size_t read_offs;
    for (int i = 0; i < 20; ++i)
    {
        read_offs = 0;

        ZyanStatus status;
        ZydisDecodedInstruction info;
        while ((status = ZydisDecoderDecodeBuffer(
            &decoder,
            xul_code + read_offs,
            xul_code_len - read_offs,
            &info
        )) != ZYDIS_STATUS_NO_MORE_DATA)
        {
            if (!ZYAN_SUCCESS(status))
            {
                ++read_offs;
                ++num_bad_insn;
                continue;
            }

#ifndef DISAS_BENCH_NO_FORMAT
            char printBuffer[256];
            ZydisFormatterFormatInstruction(
                &formatter, &info, printBuffer, sizeof(printBuffer), read_offs
            );
#endif

            read_offs += info.length;
            ++num_valid_insns;
        }
    }

    printf(
        "Disassembled %zu instructions (%zu valid, %zu bad)\n",
        num_valid_insns + num_bad_insn,
        num_valid_insns,
        num_bad_insn
    );

    free(xul_code);
    return 0;
}
