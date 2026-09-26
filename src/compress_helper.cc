#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <cstdlib>

#include <brotli/decode.h>
#include <zlib.h>
#include <zstd.h>

#include <QScopeGuard>

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
    /// windowBits can also be greater than 15 for optional gzip encoding. Add 16
    /// to windowBits to write a simple gzip header and trailer around the
    /// compressed data instead of a zlib wrapper.
    if (inflateInit2(&strm, MAX_WBITS | 16) != Z_OK) {
        if (ok)
            *ok = false;
        return { };
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
                Q_FALLTHROUGH();
            case Z_DATA_ERROR:
                Q_FALLTHROUGH();
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                if (ok)
                    *ok = false;
                return { };
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
            return { };
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
        return { };
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
                Q_FALLTHROUGH();
            case Z_DATA_ERROR:
                Q_FALLTHROUGH();
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                if (ok)
                    *ok = false;
                return { };
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
        return { };
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
                Q_FALLTHROUGH();
            case Z_DATA_ERROR:
                Q_FALLTHROUGH();
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                if (ok)
                    *ok = false;
                return { };
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

QByteArray uncompressZstd(const QByteArray &src, bool *ok)
{
    auto err = [&ok]() -> QByteArray {
        if (ok)
            *ok = false;
        return { };
    };

    /// \see https://github.com/facebook/zstd/blob/dev/examples/streaming_decompression.c
    const size_t buf_in_size = ZSTD_DStreamInSize();
    void *const buf_in = std::malloc(buf_in_size);
    if (buf_in == nullptr)
        return err();
    QScopeGuard guard_buf_in{ [buf_in]() { std::free(buf_in); } };

    const size_t buf_out_size = ZSTD_DStreamOutSize();
    void *const buf_out = std::malloc(buf_out_size);
    if (buf_out == nullptr)
        return err();
    QScopeGuard guard_buf_out{ [buf_out]() { std::free(buf_out); } };

    ZSTD_DCtx *const dctx = ZSTD_createDCtx();
    if (dctx == nullptr)
        return err();
    QScopeGuard guard_dctx{ [dctx]() { ZSTD_freeDCtx(dctx); } };

    const int block_count = static_cast<int>((std::size(src) + buf_in_size - 1) / buf_in_size);

    QByteArray res;

    for (int i = 0; i < block_count; ++i) {
        const size_t block_length = (std::min)(buf_in_size, static_cast<size_t>(std::size(src)) - i * buf_in_size);
        std::memcpy(buf_in, src.data() + i * buf_in_size, block_length);

        ZSTD_inBuffer input{ buf_in, static_cast<size_t>(block_length), 0 };
        while (input.pos < input.size) {
            ZSTD_outBuffer output{ buf_out, buf_out_size, 0 };
            const size_t ret = ZSTD_decompressStream(dctx, &output, &input);
            // 最后一个 frame 完成时会返回 0，因为我们有完整的数据，所以一定不会将 frame 分开，
            // 此处若非零说明后面还有一部分的 frame 这是不可能发生的，直接返回错误
            if (ZSTD_isError(ret))
                return err();
            res.append(reinterpret_cast<char *>(buf_out), static_cast<qsizetype>(output.pos));
        }
    }

    if (ok)
        *ok = true;
    return res;
}
