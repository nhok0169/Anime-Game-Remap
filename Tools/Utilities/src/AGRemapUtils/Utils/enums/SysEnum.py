from .StrEnum import StrEnum


# SysEnum: Enum for the types of systems to test
class SysEnum(StrEnum):
    API = "api"
    Script = "script"