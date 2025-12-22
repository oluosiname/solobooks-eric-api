import typing as _typing
import platform as _platform

_ENCODING = 'utf-8'


def decode(to_decode: _typing.Optional[bytes])->_typing.Union[str]:
    if type(to_decode) is bytes:
        return to_decode.decode(encoding=_ENCODING)
    else:
        raise TypeError("Input must be bytes.")


def encode_ifstr(to_encode: _typing.Union[bytes, str, None])->_typing.Union[bytes, None]:
    """Encode to_encode if of type str"""
    if to_encode is None:
        return None
    if type(to_encode) is bytes:
        return to_encode
    elif type(to_encode) is str:
        return to_encode.encode(encoding=_ENCODING)
    else:
        raise TypeError("Input must be string, bytes or None")


def get_platform():
    return _platform.system()


def get_lib_name(name: str):
    platsys = get_platform()
    if platsys == 'Windows':
        return name + '.dll'
    elif platsys == 'Darwin':
        return 'lib' + name + '.dylib'
    else:
        return 'lib' + name + '.so'

