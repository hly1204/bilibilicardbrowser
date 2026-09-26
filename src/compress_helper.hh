#ifndef COMPRESS_HELPER_HH
#define COMPRESS_HELPER_HH

#include <QAnyStringView>
#include <QByteArray>
#include <QtCompilerDetection>

QByteArray uncompressGzip(const QByteArray &src, bool *ok = nullptr);

QByteArray uncompressBrotli(const QByteArray &src, bool *ok = nullptr);

QByteArray uncompressDeflate(const QByteArray &src, bool *ok = nullptr);

QByteArray uncompressZlib(const QByteArray &src, bool *ok = nullptr);

QByteArray uncompressZstd(const QByteArray &src, bool *ok = nullptr);

Q_REQUIRED_RESULT
inline QByteArray uncompress(const QByteArray &src, QAnyStringView encoding, bool *ok)
{
    if (encoding == "gzip")
        return uncompressGzip(src, ok);

    if (encoding == "br")
        return uncompressBrotli(src, ok);

    if (encoding == "deflate")
        return uncompressDeflate(src, ok);

    if (encoding == "zstd")
        return uncompressZstd(src, ok);

    // undefined
    if (ok)
        *ok = false;
    return { };
}

#endif
