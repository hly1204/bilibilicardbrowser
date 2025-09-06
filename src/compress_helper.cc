#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iterator>

#include <brotli/decode.h>
#include <zlib.h>

#include "compress_helper.hh"

QByteArray uncompressGzip(const QByteArray &src, bool *ok)
{
    z_stream strm;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    /// \see https://www.zlib.net/manual.html
    // windowBits can also be greater than 15 for optional gzip encoding. Add 16
    // to windowBits to write a simple gzip header and trailer around the
    // compressed data instead of a zlib wrapper.
    if (inflateInit2(&strm, MAX_WBITS | 16) != Z_OK) {
        if (ok)
            *ok = false;
        return {};
    }

    enum { CHUNK = 16384 };
    Bytef in[CHUNK];
    Bytef out[CHUNK];

    const int block_count = static_cast<int>(std::size(src) + CHUNK - 1) / CHUNK;

    QByteArray res;

    for (int i = 0; i < block_count; ++i) {
        const int block_length = std::min<int>(CHUNK, static_cast<int>(std::size(src) - i * CHUNK));
        std::memcpy(in, src.data() + i * CHUNK, block_length);
        strm.avail_in = block_length;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            const int ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);
            switch (ret) {
            case Z_NEED_DICT:
                [[fallthrough]];
            case Z_DATA_ERROR:
                [[fallthrough]];
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                if (ok)
                    *ok = false;
                return {};
            default:
                break;
            }
            res.append(reinterpret_cast<char *>(out), CHUNK - strm.avail_out);
        } while (strm.avail_out == 0);
    }

    inflateEnd(&strm);

    if (ok)
        *ok = true;
    return res;
}

QByteArray uncompressBrotli(const QByteArray &src, bool *ok)
{
    BrotliDecoderState *state = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);

    enum { CHUNK = 16384 };
    std::uint8_t out[CHUNK];

    std::size_t available_in = std::size(src);
    const std::uint8_t *next_in = reinterpret_cast<const std::uint8_t *>(src.data());

    QByteArray res;

    for (;;) {
        std::size_t available_out = CHUNK;
        std::uint8_t *next_out = out;

        const BrotliDecoderResult result = BrotliDecoderDecompressStream(
                state, &available_in, &next_in, &available_out, &next_out, nullptr);
        if (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT
            || result == BROTLI_DECODER_RESULT_SUCCESS) {
            res.append(reinterpret_cast<char *>(out),
                       static_cast<qsizetype>(CHUNK - available_out));
            if (result == BROTLI_DECODER_RESULT_SUCCESS) {
                break;
            }
        }

        if (result == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT
            || result == BROTLI_DECODER_RESULT_ERROR) {
            BrotliDecoderDestroyInstance(state);
            if (ok) {
                *ok = false;
            }
            return {};
        }
    }

    BrotliDecoderDestroyInstance(state);

    if (ok)
        *ok = true;
    return res;
}

QByteArray uncompressDeflate(const QByteArray &src, bool *ok)
{
    z_stream strm;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    /// \see https://www.zlib.net/manual.html
    /// windowBits can also be –8..–15 for raw deflate. In this case,
    /// -windowBits determines the window size. deflate() will then generate raw
    /// deflate data with no zlib header or trailer, and will not compute a
    /// check value.
    if (inflateInit2(&strm, -MAX_WBITS) != Z_OK) {
        if (ok)
            *ok = false;
        return {};
    }

    enum { CHUNK = 16384 };
    Bytef in[CHUNK];
    Bytef out[CHUNK];

    const int block_count = static_cast<int>(std::size(src) + CHUNK - 1) / CHUNK;

    QByteArray res;

    for (int i = 0; i < block_count; ++i) {
        const int block_length = std::min<int>(CHUNK, static_cast<int>(std::size(src) - i * CHUNK));
        std::memcpy(in, src.data() + i * CHUNK, block_length);
        strm.avail_in = block_length;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            const int ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);
            switch (ret) {
            case Z_NEED_DICT:
                [[fallthrough]];
            case Z_DATA_ERROR:
                [[fallthrough]];
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                if (ok)
                    *ok = false;
                return {};
            default:
                break;
            }
            res.append(reinterpret_cast<char *>(out), CHUNK - strm.avail_out);
        } while (strm.avail_out == 0);
    }

    inflateEnd(&strm);

    if (ok)
        *ok = true;
    return res;
}

QByteArray uncompressZlib(const QByteArray &src, bool *ok)
{
    z_stream strm;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    /// \see https://www.zlib.net/manual.html
    if (inflateInit2(&strm, MAX_WBITS) != Z_OK) {
        if (ok)
            *ok = false;
        return {};
    }

    enum { CHUNK = 16384 };
    Bytef in[CHUNK];
    Bytef out[CHUNK];

    const int block_count = static_cast<int>(std::size(src) + CHUNK - 1) / CHUNK;

    QByteArray res;

    for (int i = 0; i < block_count; ++i) {
        const int block_length = std::min<int>(CHUNK, static_cast<int>(std::size(src) - i * CHUNK));
        std::memcpy(in, src.data() + i * CHUNK, block_length);
        strm.avail_in = block_length;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            const int ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);
            switch (ret) {
            case Z_NEED_DICT:
                [[fallthrough]];
            case Z_DATA_ERROR:
                [[fallthrough]];
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                if (ok)
                    *ok = false;
                return {};
            default:
                break;
            }
            res.append(reinterpret_cast<char *>(out), CHUNK - strm.avail_out);
        } while (strm.avail_out == 0);
    }

    inflateEnd(&strm);

    if (ok)
        *ok = true;
    return res;
}