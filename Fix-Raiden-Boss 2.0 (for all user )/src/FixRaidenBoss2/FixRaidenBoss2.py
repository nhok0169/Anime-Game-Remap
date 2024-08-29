# ===== Raiden Boss Fix =====
# Authors: NK#1321, Albert Gold#2696
#
# if you used it to fix your raiden pls give credit for "Nhok0169" and "Albert Gold#2696"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)


import os
import shutil
import configparser
import re
import struct
import traceback
from typing import List, Callable, Optional, Union, Dict, Any, TypeVar, Hashable, Tuple, Set, DefaultDict, Generic, Type
from collections import deque, defaultdict, OrderedDict
from functools import cmp_to_key
from enum import Enum
import argparse
import ntpath


T = TypeVar('T')
N = TypeVar('N')
Pattern = TypeVar('Pattern')
TextIoWrapper = TypeVar('TextIoWrapper')

# change our current working directory to this file, allowing users to run program
#   by clicking on the script instead of running by CLI
if __name__ == "__main__":
    os.chdir(os.path.dirname(os.path.abspath(__file__)))


class FileExt(Enum):
    Ini = ".ini"
    Txt = ".txt"
    Buf = ".buf"


class FileTypes(Enum):
    Default = "file"
    Ini = f"*{FileExt.Ini.value} file"
    Blend = f"Blend{FileExt.Buf.value}"

DefaultPath = os.getcwd()
CurrentDir = "."
IniExtLen = len(FileExt.Ini.value)
MergedFile = f"merged{FileExt.Ini.value}"
OldBackupFilePrefix = "DISABLED_BossFixBackup_"
BackupFilePrefix = "DISABLED_RemapBackup_"
DuplicateFilePrefix = "DISABLED_RSDup_"
LogFile = f"RemapFixLog{FileExt.Txt.value}"

RemapBlendFile = f"Remap{FileTypes.Blend.value}"
IniFileEncoding = "utf-8"
ReadEncodings = [IniFileEncoding, "latin1"]

Deprecated = "DEPRECATED"
DefaultCacheSize = 128

DeleteBackupOpt = '--deleteBackup'
FixOnlyOpt = '--fixOnly'
RevertOpt = '--revert'
AllOpt = '--all'
TypeOpt = "--types"
VersionOpt = "--version"


class VGRemap():
    """
    Class for handling the vertex group remaps for mods

    Parameters
    ----------
    vgRemap: Dict[:class:`int`, :class:`int`] 
        The vertex group remap from one type of mod to another
    """

    def __init__(self, vgRemap: Dict[int, int]):
        self._maxIndex = 0
        self.remap = vgRemap

    @property
    def remap(self):
        """
        The vertex group remap

        :getter: Retrieves the remap
        :setter: Sets a new remap
        :type: Dict[:class:`int`, :class:`int`]
        """

        return self._remap

    @remap.setter
    def remap(self, newVgRemap: Dict[int, int]):
        self._remap = newVgRemap
        if (self._remap):
            self._maxIndex = max(list(self._remap.keys()))
        else:
            self._maxIndex = None

    @property
    def maxIndex(self):
        """
        The maximum index in the vertex group remap

        :getter: Retrieves the max index
        :type: :class:`int`
        """

        return self._maxIndex


##### LINK HashData #####
HashData = HashData = {4.0 : {"Amber": {"draw_vb": "870a7499", "position_vb": "caddc4c6", "blend_vb": "ca5bd26e", "texcoord_vb": "e3047676", "ib": "9976d124",
                 "tex_head_diffuse": "ae27902d", "tex_head_lightmap": "29b001ba", "tex_head_shadowramp": "7eb5b84e",
                 "tex_body_diffuse": "bc86882f", "tex_body_lightmap": "9e1294dd", "tex_body_metalmap": "b0e08915", "tex_body_shadowramp": "7eb5b84e",
                 "tex_face_diffuse": "1d064079", "tex_face_lightmap": "4e3376db", "tex_face_shadow": "3f396398", "tex_face_shadowramp": "7eb5b84e"},
        "AmberCN": {"draw_vb": "da0adf2f", "position_vb": "7f94e8da", "blend_vb": "f35340d5", "texcoord_vb": "dbc594b6", "ib": "8cc9274b",
                    "tex_head_diffuse": "ae27902d", "tex_head_lightmap": "29b001ba", "tex_head_shadowramp": "7eb5b84e",
                    "tex_body_diffuse": "f683bcac", "tex_body_lightmap": "69b6e698", "tex_body_metalmap": "b0e08915", "tex_body_shadowramp": "7eb5b84e",
                    "tex_face_diffuse": "1d064079", "tex_face_lightmap": "4e3376db", "tex_face_shadow": "3f396398", "tex_face_shadowramp": "7eb5b84e"},
        "Jean": {"draw_vb": "e6055135", "position_vb": "191af650", "blend_vb": "3cb8153c", "texcoord_vb": "1722136c", "ib": "29835d20",
                 "tex_head_diffuse": "dba2791d", "tex_head_lightmap": "0bd77e81", "tex_head_shadowramp": "7eb5b84e",
                 "tex_body_diffuse": "d1ae8efe", "tex_body_lightmap": "cee17ba5", "tex_body_metalmap": "b0e08915", "tex_body_shadowramp": "7eb5b84e",
                 "tex_face_diffuse": "c2d1a57e", "tex_face_lightmap": "4e3376db", "tex_face_shadow": "bf9fccca", "tex_face_shadowramp": "7eb5b84e"},
        "JeanCN": {"draw_vb": "2a29e333", "position_vb": "93bb2522", "blend_vb": "d159bf31", "texcoord_vb": "0ffefb98", "ib": "920c0b3f",
                   "tex_head_diffuse": "6eca0f93", "tex_head_lightmap": "92ed604c", "tex_head_shadowramp": "7eb5b84e",
                   "tex_body_diffuse": "0f9c7705", "tex_body_lightmap": "617c45a0", "tex_body_metalmap": "b0e08915", "tex_body_shadowramp": "7eb5b84e",
                   "tex_face_diffuse": "c2d1a57e", "tex_face_lightmap": "4e3376db", "tex_face_shadow": "bf9fccca", "tex_face_shadowramp": "7eb5b84e"},
        "Keqing": {"draw_vb": "4526145e", "position_vb": "3aaf3e94", "blend_vb": "0bf8e621", "texcoord_vb": "723848fe", "ib": "f325e394",
                   "tex_head_diffuse": "58de714b", "tex_head_lightmap": "da3e4a28", "tex_head_metalmap": "b0e08915", "tex_head_shadowramp": "7eb5b84e",
                   "tex_body_diffuse": "874b8c0b", "tex_body_lightmap": "0695efb7", "tex_body_metalmap": "b0e08915", "tex_body_shadowramp": "7eb5b84e",
                   "tex_dress_diffuse": "874b8c0b", "tex_dress_lightmap": "0695efb7", "tex_dress_metalmap": "b0e08915", "tex_dress_shadowramp": "7eb5b84e",
                   "tex_face_diffuse": "d8c9c399", "tex_face_lightmap": "4e3376db", "tex_face_shadow": "3f396398", "tex_face_shadowramp": "7eb5b84e"},
        "KeqingOpulent": {"draw_vb": "efcc8769", "position_vb": "0d7e3cc5", "blend_vb": "6f010b58", "texcoord_vb": "52f78cb7", "ib": "44bba21c",
                   "tex_head_diffuse": "e2d7ae66", "tex_head_lightmap": "13e2b0ab", "tex_head_metalmap": "b0e08915", "tex_head_shadowramp": "7eb5b84e",
                   "tex_body_diffuse": "2af5bf71", "tex_body_lightmap": "195af53a", "tex_body_metalmap": "b0e08915", "tex_body_shadowramp": "7eb5b84e",
                   "tex_face_diffuse": "c2b17f84", "tex_face_lightmap": "4e3376db", "tex_face_shadow": "3f396398", "tex_face_shadowramp": "7eb5b84e"},
        "Mona": {"draw_vb": "00741928", "position_vb": "20d0bfab", "blend_vb": "52f0e9a0", "texcoord_vb": "a8191396", "ib": "ef876207",
                 "tex_head_diffuse": "b518c5a5", "tex_head_lightmap": "0c679d22", "tex_head_metalmap": "b0e08915", "tex_head_shadowramp": "7eb5b84e",
                 "tex_body_diffuse": "5f873d89", "tex_body_lightmap": "29d50a21", "tex_body_metalmap": "b0e08915", "tex_body_shadowramp": "7eb5b84e",
                 "tex_face_diffuse": "8e116301", "tex_face_lightmap": "4e3376db", "tex_face_shadow": "bf9fccca", "tex_face_shadowramp": "7eb5b84e"},
        "MonaCN": {"draw_vb": "41f18240", "position_vb": "ee5ed1dc", "blend_vb": "bad2731b", "texcoord_vb": "e543af5d", "ib": "ed79ea5b",
                   "tex_head_diffuse": "0320a4d2", "tex_head_lightmap": "df0f8b90", "tex_head_metalmap": "b0e08915", "tex_head_shadowramp": "7eb5b84e",
                   "tex_body_diffuse": "c043f913", "tex_body_lightmap": "a3369d08", "tex_body_metalmap": "b0e08915", "tex_body_shadowramp": "7eb5b84e",
                   "tex_face_diffuse": "8e116301", "tex_face_lightmap": "4e3376db", "tex_face_shadow": "bf9fccca", "tex_face_shadowramp": "7eb5b84e"},
        "Ningguang": {"draw_vb": "e4fc5902", "position_vb": "55b43e99", "blend_vb": "9f7dc19c", "texcoord_vb": "906ad233", "ib": "93085db7",
                   "tex_head_diffuse": "e0789f0d", "tex_head_lightmap": "5d182ae7", "tex_head_shadowramp": "7eb5b84e",
                   "tex_body_diffuse": "5ffe95c2", "tex_body_lightmap": "64e6b893", "tex_body_metalmap": "b0e08915", "tex_body_shadowramp": "7eb5b84e",
                   "tex_dress_diffuse": "5ffe95c2", "tex_dress_lightmap": "64e6b893", "tex_dress_shadowramp": "7eb5b84e",
                   "tex_face_diffuse": "4cc85338", "tex_face_lightmap": "4e3376db", "tex_face_shadow": "bf9fccca", "tex_face_shadowramp": "7eb5b84e"},
        "NingguangOrchid": {"draw_vb": "10de9c78", "position_vb": "db37b198", "blend_vb": "a8246d4a", "texcoord_vb": "396aa3ec", "ib": "f1d09b47",
                   "tex_head_diffuse": "b68d7488", "tex_head_lightmap": "bc1034dd", "tex_head_metalmap": "b0e08915", "tex_head_shadowramp": "7eb5b84e",
                   "tex_body_diffuse": "a4597b85", "tex_body_lightmap": "0e26784e", "tex_body_metalmap": "b0e08915", "tex_body_shadowramp": "7eb5b84e",
                   "tex_dress_diffuse": "a4597b85", "tex_dress_lightmap": "0e26784e", "tex_dress_metalmap": "b0e08915", "tex_dress_shadowramp": "7eb5b84e",
                   "tex_face_diffuse": "4cc85338", "tex_face_lightmap": "4e3376db", "tex_face_shadow": "bf9fccca", "tex_face_shadowramp": "7eb5b84e"},
        "Raiden": {"draw_vb": "a05e7bec", "position_vb": "e48c61f3", "blend_vb": "1a495487", "texcoord_vb": "0c37fc86", "ib": "428c56cd"},
        "RaidenBoss": {"blend_vb": "fe5c0180"},
        "Rosaria": {"draw_vb": "9e1868d9", "position_vb": "748f40a5", "blend_vb": "4de959bd", "texcoord_vb": "06b8fbf5", "ib": "5d18b9d6",
                    "tex_head_diffuse": "81b2d0ca", "tex_head_lightmap": "2f19c547", "tex_head_metalmap": "b0e08915", "tex_head_shadowramp": "7eb5b84e",
                    "tex_body_diffuse": "9abde85f", "tex_body_lightmap": "743ffc09", "tex_body_metalmap": "b0e08915", "tex_body_shadowramp": "7eb5b84e",
                    "tex_dress_diffuse": "81b2d0ca", "tex_dress_lightmap": "2f19c547", "tex_dress_metalmap": "b0e08915", "tex_dress_shadowramp": "7eb5b84e",
                    "tex_extra_diffuse": "9abde85f", "tex_extra_lightmap": "743ffc09", "tex_extra_metalmap": "b0e08915", "tex_extra_shadowramp": "7eb5b84e",
                    "tex_face_diffuse": "2abd61ee", "tex_face_lightmap": "4e3376db", "tex_face_shadow": "bf9fccca", "tex_face_shadowramp": "7eb5b84e"},
        "RosariaCN": {"draw_vb": "f3d4a01a", "position_vb": "59a1f8b1", "blend_vb": "a7bee046", "texcoord_vb": "86e0d16b", "ib": "851e4de1",
                      "tex_head_diffuse": "55280cb0", "tex_head_lightmap": "825c32a0", "tex_head_metalmap": "b0e08915", "tex_head_shadowramp": "7eb5b84e",
                      "tex_body_diffuse": "bd6fcf34", "tex_body_lightmap": "cf7b6deb", "tex_body_metalmap": "b0e08915", "tex_body_shadowramp": "7eb5b84e",
                      "tex_dress_diffuse": "55280cb0", "tex_dress_lightmap": "825c32a0", "tex_dress_metalmap": "b0e08915", "tex_dress_shadowramp": "7eb5b84e",
                      "tex_extra_diffuse": "bd6fcf34", "tex_extra_lightmap": "cf7b6deb", "tex_extra_metalmap": "b0e08915", "tex_extra_shadowramp": "7eb5b84e",
                      "tex_face_diffuse": "2abd61ee", "tex_face_lightmap": "4e3376db", "tex_face_shadow": "bf9fccca", "tex_face_shadowramp": "7eb5b84e"}},
4.1 : {"Amber": {"draw_vb":"0eef5bbe"},
       "AmberCN": {"draw_vb":"53eff008"},
       "Jean": {"draw_vb":"6fe07e12"},
       "JeanCN": {"draw_vb":"a3cccc14"},
       "Keqing": {"draw_vb": "ccc33b79"},
       "KeqingOpulent": {"draw_vb": "6629a84e"},
       "Mona": {"draw_vb":"8991360f"},
       "MonaCN": {"draw_vb":"c814ad67"},
       "Ningguang": {"draw_vb": "6d197625"},
       "NingguangOrchid": {"draw_vb": "993bb35f"},
       "Raiden": {"draw_vb":"29bb54cb"},
       "Rosaria": {"draw_vb":"17fd47fe"},
       "RosariaCN": {"draw_vb":"7a318f3d"}},
4.3 : {"Amber": {"ib":"a1a2bbfb"},
       "AmberCN": {"ib":"b41d4d94"},
       "Jean": {"ib":"115737ff"},
       "JeanCN": {"ib":"aad861e0"},
       "Keqing": {"ib": "cbf1894b"},
       "KeqingOpulent": {"ib": "7c6fc8c3"},
       "Mona": {"ib":"d75308d8"},
       "MonaCN": {"ib":"d5ad8084"},
       "Ningguang": {"ib": "abdc3768"},
       "NingguangOrchid": {"ib":"c904f198"},
       "Raiden": {"ib":"7a583c12"},
       "Rosaria": {"ib":"65ccd309"},
       "RosariaCN": {"ib":"bdca273e"}},
4.4 : {"Amber": {"position_vb": "a2ea4b2d", "blend_vb": "36d20a67", "texcoord_vb": "81b777ca", "ib": "b03c7e30"},
       "AmberCN": {"position_vb": "557b2eff"},
       "Mona": {"position_vb": "7a1dc890", "blend_vb": "b043715a"},
       "MonaCN": {"position_vb": "515f3ce6"},
       "Ningguang": {"draw_vb": "4c2f9a0a", "position_vb": "f9e1b52b", "blend_vb": "735eaea4", "texcoord_vb": "1f0ab400", "ib": "ad75352c"}},
4.6 : {"Arlecchino" : {"draw_vb": "44e3487a", "position_vb": "6895f405", "blend_vb": "e211de60", "texcoord_vb": "8b17a419", "ib": "e811d2a1"}}}
##### ENDLINK ############

##### LINK IndexData #######
IndexData = {4.0 : {"Amber": {"head": "0", "body": "5670"},
        "AmberCN": {"head": "0", "body": "5670"},
        "Jean": {"head": "0", "body": "7779"},
        "JeanCN": {"head": "0", "body": "7779"},
        "Mona": {"head": "0", "body": "17688"},
        "MonaCN": {"head": "0", "body": "17688"},
        "Ningguang": {"head": "0", "body": "12384", "dress": "47157"},
        "NingguangOrchid": {"head": "0", "body": "43539", "dress": "56124"},
        "Rosaria": {"head": "0", "body": "11139", "dress": "44088", "extra": "45990"},
        "RosariaCN": {"head": "0", "body": "11025", "dress": "46539", "extra": "48441"},
        "Keqing": {"head": "0", "body": "10824", "dress": "48216"},
        "KeqingOpulent": {"head": "0", "body": "19623"}}}
##### ENDLINK ############

##### LINK VGRemapData #######
VGRemapData = {4.0: { "Amber" : {"AmberCN": VGRemap({0: 7, 1: 6, 2: 9, 3: 10, 4: 11, 5: 29, 6: 8, 7: 12, 8: 13, 9: 14, 10: 15, 11: 16, 12: 17, 
                               13: 77, 14: 1, 15: 0, 16: 73, 17: 18, 18: 19, 19: 20, 20: 21, 21: 53, 22: 70, 23: 74, 24: 50, 
                               25: 30, 26: 47, 27: 51, 28: 76, 29: 75, 30: 24, 31: 71, 32: 28, 33: 27, 34: 54, 35: 52, 36: 31, 
                               37: 72, 38: 55, 39: 56, 40: 61, 41: 58, 42: 62, 43: 64, 44: 65, 45: 67, 46: 68, 47: 57, 48: 59, 49: 60, 
                               50: 63, 51: 66, 52: 69, 53: 48, 54: 26, 55: 25, 56: 49, 57: 32, 58: 33, 59: 38, 60: 35, 61: 39, 62: 41, 
                               63: 42, 64: 44, 65: 45, 66: 34, 67: 36, 68: 37, 69: 40, 70: 43, 71: 46, 72: 22, 73: 23, 74: 2, 75: 3, 76: 4, 77: 5})},
        "AmberCN": {"Amber" : VGRemap({0: 15, 1: 14, 2: 74, 3: 75, 4: 76, 5: 77, 6: 1, 7: 0, 8: 6, 9: 2, 10: 3, 11: 4, 12: 7, 13: 8, 14: 9, 15: 10, 
                               16: 11, 17: 12, 18: 17, 19: 18, 20: 19, 21: 20, 22: 72, 23: 73, 24: 30, 25: 55, 26: 54, 27: 33, 28: 32, 
                               29: 5, 30: 25, 31: 36, 32: 57, 33: 58, 34: 66, 35: 60, 36: 67, 37: 68, 38: 59, 39: 61, 40: 69, 41: 62, 42: 
                               63, 43: 70, 44: 64, 45: 65, 46: 71, 47: 26, 48: 53, 49: 56, 50: 24, 51: 27, 52: 35, 53: 21, 54: 34, 55: 38, 
                               56: 39, 57: 47, 58: 41, 59: 48, 60: 49, 61: 40, 62: 42, 63: 50, 64: 43, 65: 44, 66: 51, 67: 45, 68: 46, 69: 52, 
                               70: 22, 71: 31, 72: 37, 73: 16, 74: 23, 75: 29, 76: 28, 77: 13})},
        "Jean" : {"JeanCN": VGRemap({0: 50, 1: 102, 2: 103, 3: 104, 4: 79, 5: 56, 6: 24, 7: 25, 8: 33, 9: 34, 10: 35, 11: 30, 12: 31, 13: 32, 14: 26, 
                             15: 27, 16: 28, 17: 29, 18: 58, 19: 75, 20: 76, 21: 59, 22: 60, 23: 61, 24: 62, 25: 63, 26: 64, 27: 65, 28: 66, 
                             29: 67, 30: 68, 31: 69, 32: 70, 33: 71, 34: 72, 35: 73, 36: 52, 37: 51, 38: 6, 39: 7, 40: 10, 41: 11, 42: 12, 
                             43: 13, 44: 2, 45: 3, 46: 81, 47: 98, 48: 99, 49: 82, 50: 83, 51: 84, 52: 85, 53: 86, 54: 87, 55: 88, 56: 89, 
                             57: 90, 58: 91, 59: 92, 60: 93, 61: 94, 62: 95, 63: 96, 64: 53, 65: 54, 66: 4, 67: 5, 68: 16, 69: 17, 70: 14, 
                             71: 15, 72: 8, 73: 9, 74: 19, 75: 18, 76: 0, 77: 1, 78: 21, 79: 23, 80: 20, 81: 22, 82: 47, 83: 48, 84: 49, 85: 43, 
                             86: 44, 87: 45, 88: 46, 89: 40, 90: 41, 91: 42, 92: 36, 93: 37, 94: 38, 95: 39, 96: 55, 97: 77, 98: 57, 99: 74, 
                             100: 78, 101: 100, 102: 80, 103: 97, 104: 101})},
        "JeanCN": {"Jean": VGRemap({0: 76, 1: 77, 2: 44, 3: 45, 4: 66, 5: 67, 6: 38, 7: 39, 8: 72, 9: 73, 10: 40, 11: 41, 12: 42, 13: 43, 14: 70, 15: 71, 
                            16: 68, 17: 69, 18: 75, 19: 74, 20: 80, 21: 78, 22: 81, 23: 79, 24: 6, 25: 7, 26: 14, 27: 15, 28: 16, 29: 17, 30: 11, 
                            31: 12, 32: 13, 33: 8, 34: 9, 35: 10, 36: 92, 37: 93, 38: 94, 39: 95, 40: 89, 41: 90, 42: 91, 43: 85, 44: 86, 45: 87, 
                            46: 88, 47: 82, 48: 83, 49: 84, 50: 0, 51: 37, 52: 36, 53: 64, 54: 65, 55: 96, 56: 5, 57: 98, 58: 18, 59: 21, 60: 22, 
                            61: 23, 62: 24, 63: 25, 64: 26, 65: 27, 66: 28, 67: 29, 68: 30, 69: 31, 70: 32, 71: 33, 72: 34, 73: 35, 74: 99, 75: 19, 
                            76: 20, 77: 97, 78: 100, 79: 4, 80: 102, 81: 46, 82: 49, 83: 50, 84: 51, 85: 52, 86: 53, 87: 54, 88: 55, 89: 56, 90: 57, 
                            91: 58, 92: 59, 93: 60, 94: 61, 95: 62, 96: 63, 97: 103, 98: 47, 99: 48, 100: 101, 101: 104, 102: 1, 103: 2, 104: 3})},
        "Mona": {"MonaCN": VGRemap({0: 37, 1: 38, 2: 39, 3: 36, 4: 58, 5: 30, 6: 31, 7: 32, 8: 29, 9: 57, 10: 26, 11: 25, 12: 24, 13: 27, 14: 28, 15: 34, 
                            16: 35, 17: 40, 18: 33, 19: 81, 20: 106, 21: 102, 22: 47, 23: 43, 24: 46, 25: 44, 26: 42, 27: 41, 28: 45, 29: 105, 30: 104, 
                            31: 60, 32: 54, 33: 53, 34: 20, 35: 83, 36: 56, 37: 55, 38: 21, 39: 15, 40: 10, 41: 16, 42: 11, 43: 5, 44: 23, 45: 0, 46: 6, 
                            47: 1, 48: 22, 49: 77, 50: 49, 51: 50, 52: 100, 53: 51, 54: 52, 55: 79, 56: 2, 57: 7, 58: 3, 59: 4, 60: 8, 61: 9, 62: 12, 
                            63: 17, 64: 13, 65: 14, 66: 18, 67: 19, 68: 59, 69: 76, 70: 82, 71: 99, 72: 80, 73: 103, 74: 48, 75: 101, 76: 84, 77: 90, 
                            78: 87, 79: 93, 80: 96, 81: 97, 82: 85, 83: 86, 84: 88, 85: 89, 86: 91, 87: 92, 88: 94, 89: 95, 90: 98, 91: 78, 92: 61, 
                            93: 67, 94: 64, 95: 70, 96: 73, 97: 74, 98: 62, 99: 63, 100: 65, 101: 66, 102: 68, 103: 69, 104: 71, 105: 72, 106: 75})},
        "MonaCN": {"Mona": VGRemap({0: 45, 1: 47, 2: 56, 3: 58, 4: 59, 5: 43, 6: 46, 7: 57, 8: 60, 9: 61, 10: 40, 11: 42, 12: 62, 13: 64, 14: 65, 15: 39, 
                            16: 41, 17: 63, 18: 66, 19: 67, 20: 34, 21: 38, 22: 48, 23: 44, 24: 12, 25: 11, 26: 10, 27: 13, 28: 14, 29: 8, 30: 5, 
                            31: 6, 32: 7, 33: 18, 34: 15, 35: 16, 36: 3, 37: 0, 38: 1, 39: 2, 40: 17, 41: 27, 42: 26, 43: 23, 44: 25, 45: 28, 46: 24, 
                            47: 22, 48: 74, 49: 50, 50: 51, 51: 53, 52: 54, 53: 33, 54: 32, 55: 37, 56: 36, 57: 9, 58: 4, 59: 68, 60: 31, 61: 92, 
                            62: 98, 63: 99, 64: 94, 65: 100, 66: 101, 67: 93, 68: 102, 69: 103, 70: 95, 71: 104, 72: 105, 73: 96, 74: 97, 75: 106, 
                            76: 69, 77: 49, 78: 91, 79: 55, 80: 72, 81: 19, 82: 70, 83: 35, 84: 76, 85: 82, 86: 83, 87: 78, 88: 84, 89: 85, 90: 77, 
                            91: 86, 92: 87, 93: 79, 94: 88, 95: 89, 96: 80, 97: 81, 98: 90, 99: 71, 100: 52, 101: 75, 102: 21, 103: 73, 104: 30, 
                            105: 29, 106: 20})},
        "Ningguang": {"NingguangOrchid": VGRemap({0: 0, 1: 1, 2: 2, 3: 3, 4: 4, 5: 5, 6: 6, 7: 7, 8: 8, 9: 9, 10: 10, 11: 11, 12: 12, 13: 13, 14: 14,
                            15: 15, 16: 16, 17: 17, 18: 18, 19: 19, 20: 20, 21: 21, 22: 22, 23: 23, 24: 24, 25: 25, 26: 26, 27: 27, 29: 28, 30: 29,
                            31: 30, 32: 31, 33: 32, 34: 33, 35: 34, 36: 35, 37: 36, 38: 37, 39: 38, 40: 39, 41: 40, 42: 41, 43: 42, 44: 43, 45: 44,
                            46: 45, 47: 46, 48: 47, 52: 48, 53: 49, 54: 50, 55: 51, 56: 52, 57: 53, 58: 54, 59: 55, 60: 56, 61: 57, 62: 58, 63: 59,
                            64: 60, 65: 61, 66: 62, 67: 63, 68: 64, 69: 65, 70: 66, 71: 67, 72: 68, 73: 69, 74: 70, 75: 71, 76: 72, 77: 73, 78: 74,
                            79: 75, 80: 76, 81: 77, 82: 78, 83: 79, 84: 80, 85: 81, 86: 82, 87: 83, 88: 84, 89: 85, 90: 86, 91: 87, 92: 88, 93: 89,
                            94: 90, 95: 91, 96: 92, 97: 93, 98: 94, 99: 95, 100: 96, 101: 97, 102: 98, 103: 99, 104: 100, 105: 101, 106: 102, 107: 103,
                            108: 104, 109: 105, 110: 106, 111: 107, 112: 108, 113: 109, 114: 110, 115: 111, 116: 112, 117: 113, 118: 114})},
        "NingguangOrchid": {"Ningguang": VGRemap({0: 0, 1: 1, 2: 2, 3: 3, 4: 4, 5: 5, 6: 6, 7: 7, 8: 8, 9: 9, 10: 10, 11: 11, 12: 12, 13: 13, 14: 14, 15: 15, 16: 16, 
                               17: 17, 18: 18, 19: 19, 20: 20, 21: 21, 22: 22, 23: 23, 24: 24, 25: 25, 26: 26, 27: 27, 28: 29, 29: 30, 30: 31, 31: 32, 
                               32: 33, 33: 34, 34: 35, 35: 36, 36: 37, 37: 38, 38: 39, 39: 40, 40: 41, 41: 42, 42: 43, 43: 44, 44: 45, 45: 46, 46: 47, 
                               47: 48, 48: 52, 49: 53, 50: 54, 51: 55, 52: 56, 53: 57, 54: 58, 55: 59, 56: 60, 57: 61, 58: 62, 59: 63, 60: 64, 61: 65, 
                               62: 66, 63: 67, 64: 68, 65: 69, 66: 70, 67: 71, 68: 72, 69: 73, 70: 74, 71: 75, 72: 76, 73: 77, 74: 78, 75: 79, 76: 80, 
                               77: 81, 78: 82, 79: 83, 80: 84, 81: 85, 82: 86, 83: 87, 84: 88, 85: 89, 86: 90, 87: 91, 88: 92, 89: 93, 90: 94, 91: 95, 
                               92: 96, 93: 97, 94: 98, 95: 99, 96: 100, 97: 101, 98: 102, 99: 103, 100: 104, 101: 105, 102: 106, 103: 107, 104: 108, 
                               105: 109, 106: 110, 107: 111, 108: 112, 109: 113, 110: 114, 111: 115, 112: 116, 113: 117, 114: 118})},
        "Rosaria": {"RosariaCN": VGRemap({0: 0, 1: 1, 2: 2, 3: 3, 4: 4, 5: 5, 6: 6, 7: 7, 8: 8, 9: 9, 10: 10, 11: 11, 12: 12, 13: 13, 14: 14, 15: 15, 
                                  16: 16, 17: 17, 18: 18, 19: 19, 20: 20, 21: 21, 22: 22, 23: 23, 24: 24, 25: 25, 26: 26, 27: 27, 28: 28, 29: 29, 
                                  30: 30, 31: 31, 32: 32, 33: 33, 34: 34, 35: 35, 36: 36, 37: 37, 38: 38, 39: 39, 40: 40, 41: 41, 42: 42, 43: 43, 
                                  44: 44, 45: 45, 46: 46, 47: 47, 48: 48, 49: 49, 50: 50, 51: 51, 52: 52, 53: 53, 54: 54, 55: 55, 56: 56, 57: 57, 
                                  58: 58, 59: 59, 60: 60, 61: 61, 62: 62, 63: 63, 64: 64, 65: 65, 66: 66, 67: 67, 68: 68, 69: 69, 70: 70, 71: 71, 
                                  72: 72, 73: 73, 74: 74, 75: 75, 76: 76, 77: 77, 78: 78, 79: 79, 80: 80, 81: 81, 82: 82, 83: 83, 84: 84, 85: 85,
                                  86: 86, 87: 87, 88: 88, 89: 89, 90: 90, 91: 91, 92: 92, 93: 93, 94: 94, 95: 95, 96: 96, 97: 97})},
        "RosariaCN": {"Rosaria": VGRemap({0: 0, 1: 1, 2: 2, 3: 3, 4: 4, 5: 5, 6: 6, 7: 7, 8: 8, 9: 9, 10: 10, 11: 11, 12: 12, 13: 13, 14: 14, 15: 15, 
                                  16: 16, 17: 17, 18: 18, 19: 19, 20: 20, 21: 21, 22: 22, 23: 23, 24: 24, 25: 25, 26: 26, 27: 27, 28: 28, 29: 29, 
                                  30: 30, 31: 31, 32: 32, 33: 33, 34: 34, 35: 35, 36: 36, 37: 37, 38: 38, 39: 39, 40: 40, 41: 41, 42: 42, 43: 43, 
                                  44: 44, 45: 45, 46: 46, 47: 47, 48: 48, 49: 49, 50: 50, 51: 51, 52: 52, 53: 53, 54: 54, 55: 55, 56: 56, 57: 57, 
                                  58: 58, 59: 59, 60: 60, 61: 61, 62: 62, 63: 63, 64: 64, 65: 65, 66: 66, 67: 67, 68: 68, 69: 69, 70: 70, 71: 71, 
                                  72: 72, 73: 73, 74: 74, 75: 75, 76: 76, 77: 77, 78: 78, 79: 79, 80: 80, 81: 81, 82: 82, 83: 83, 84: 84, 85: 85,
                                  86: 86, 87: 87, 88: 88, 89: 89, 90: 90, 91: 91, 92: 92, 93: 93, 94: 94, 95: 95, 96: 96, 97: 97})},
        "Raiden": {"RaidenBoss": VGRemap({0: 0, 1: 1, 2: 2, 3: 3, 4: 4, 5: 5, 6: 6, 7: 7, 8: 60, 9: 61, 10: 66, 11: 67,
                                12: 8, 13: 9, 14: 10, 15: 11, 16: 12, 17: 13, 18: 14, 19: 15, 20: 16, 21: 17,
                                22: 18, 23: 19, 24: 20, 25: 21, 26: 22, 27: 23, 28: 24, 29: 25, 30: 26, 31: 27,
                                32: 28, 33: 29, 34: 30, 35: 31, 36: 32, 37: 33, 38: 34, 39: 35, 40: 36, 41: 37,
                                42: 38, 43: 39, 44: 40, 45: 41, 46: 42, 47: 94, 48: 43, 49: 44, 50: 45, 51: 46,
                                52: 47, 53: 48, 54: 49, 55: 50, 56: 51, 57: 52, 58: 53, 59: 54, 60: 55, 61: 56,
                                62: 57, 63: 58, 64: 59, 65: 114, 66: 116, 67: 115, 68: 117, 69: 74, 70: 62, 71: 64,
                                72: 106, 73: 108, 74: 110, 75: 75, 76: 77, 77: 79, 78: 87, 79: 89, 80: 91, 81: 95,
                                82: 97, 83: 99, 84: 81, 85: 83, 86: 85, 87: 68, 88: 70, 89: 72, 90: 104, 91: 112,
                                92: 93, 93: 63, 94: 65, 95: 107, 96: 109, 97: 111, 98: 76, 99: 78, 100: 80, 101: 88,
                                102: 90, 103: 92, 104: 96, 105: 98, 106: 100, 107: 82, 108: 84, 109: 86, 110: 69,
                                111: 71, 112: 73, 113: 105, 114: 113, 115: 101, 116: 102, 117: 103})},
         "RaidenBoss": {"Raiden": VGRemap({0: 0, 1: 1, 2: 2, 3: 3, 4: 4, 5: 5, 6: 6, 7: 7, 8: 12, 9: 13, 10: 14, 11: 15, 12: 16, 13: 17, 14: 18, 15: 19, 16: 20, 
                                           17: 21, 18: 22, 19: 23, 20: 24, 21: 25, 22: 26, 23: 27, 24: 28, 25: 29, 26: 30, 27: 31, 28: 32, 29: 33, 30: 34, 31: 35, 
                                           32: 36, 33: 37, 34: 38, 35: 39, 36: 40, 37: 41, 38: 42, 39: 43, 40: 44, 41: 45, 42: 46, 43: 48, 44: 49, 45: 50, 46: 51,
                                           47: 52, 48: 53, 49: 54, 50: 55, 51: 56, 52: 57, 53: 58, 54: 59, 55: 60, 56: 61, 57: 62, 58: 63, 59: 64, 60: 8, 61: 9, 62: 70, 
                                           63: 93, 64: 71, 65: 94, 66: 10, 67: 11, 68: 87, 69: 110, 70: 88, 71: 111, 72: 89, 73: 112, 74: 69, 75: 75, 76: 98, 77: 76, 78: 99, 
                                           79: 77, 80: 100, 81: 84, 82: 107, 83: 85, 84: 108, 85: 86, 86: 109, 87: 78, 88: 101, 89: 79, 90: 102, 91: 80, 92: 103, 93: 92, 
                                           94: 47, 95: 81, 96: 104, 97: 82, 98: 105, 99: 83, 100: 106, 101: 115, 102: 116, 103: 117, 104: 90, 105: 113, 106: 72, 107: 95, 
                                           108: 73, 109: 96, 110: 74, 111: 97, 112: 91, 113: 114, 114: 65, 115: 67, 116: 66, 117: 68})},
         "Keqing": {"KeqingOpulent": VGRemap({0: 100, 1: 101, 2: 102, 3: 76, 4: 52, 5: 3, 6: 2, 7: 16, 8: 17, 9: 9, 10: 10, 11: 11, 12: 12,
                                    13: 13, 14: 14, 15: 15, 16: 4, 17: 5, 18: 6, 19: 7, 20: 8, 21: 54, 22: 71, 23: 72, 24: 55, 25: 56,
                                    26: 57, 27: 58, 28: 59, 29: 60, 30: 61, 31: 62, 32: 63, 33: 64, 34: 65, 35: 66, 36: 67, 37: 68, 38: 69,
                                    39: 46, 40: 47, 41: 38, 42: 39, 43: 40, 44: 41, 45: 47, 46: 47, 47: 78, 48: 95, 49: 96, 50: 79, 51: 80,
                                    52: 81, 53: 82, 54: 83, 55: 84, 56: 85, 57: 86, 58: 87, 59: 88, 60: 89, 61: 90, 62: 91, 63: 92, 64: 93,
                                    65: 48, 66: 49, 67: 42, 68: 43, 69: 44, 70: 45, 71: 49, 72: 49, 73: 1, 74: 0, 75: 50, 76: 51, 77: 28,
                                    78: 29, 79: 30, 80: 19, 81: 20, 82: 21, 83: 34, 84: 35, 85: 22, 86: 23, 87: 24, 88: 36, 89: 37, 90: 31,
                                    91: 32, 92: 33, 93: 25, 94: 26, 95: 27, 101: 73, 102: 53, 103: 70, 104: 74, 105: 97, 106: 77, 107: 94, 108: 98, 109: 18})},
         "KeqingOpulent": {"Keqing": VGRemap({0: 74, 1: 73, 2: 6, 3: 5, 4: 16, 5: 17, 6: 18, 7: 19, 8: 20, 9: 9, 10: 10, 11: 11, 12: 12, 13: 13, 14: 14, 15: 15, 16: 7, 17: 8, 18: 109, 
                                    19: 80, 20: 81, 21: 82, 22: 85, 23: 86, 24: 87, 25: 93, 26: 94, 27: 95, 28: 77, 29: 78, 30: 79, 31: 90, 32: 91, 33: 92, 34: 83, 35: 84, 
                                    36: 88, 37: 89, 38: 41, 39: 42, 40: 43, 41: 44, 42: 67, 43: 68, 44: 69, 45: 70, 46: 39, 47: 40, 48: 65, 49: 66, 50: 75, 51: 76, 52: 4, 
                                    53: 102, 54: 21, 55: 24, 56: 25, 57: 26, 58: 27, 59: 28, 60: 29, 61: 30, 62: 31, 63: 32, 64: 33, 65: 34, 66: 35, 67: 36, 68: 37, 69: 38, 
                                    70: 103, 71: 22, 72: 23, 73: 101, 74: 104, 76: 3, 77: 106, 78: 47, 79: 50, 80: 51, 81: 52, 82: 53, 83: 54, 84: 55, 85: 56, 86: 57, 87: 58, 
                                    88: 59, 89: 60, 90: 61, 91: 62, 92: 63, 93: 64, 94: 107, 95: 48, 96: 49, 97: 105, 98: 108, 100: 0, 101: 1, 102: 2})}},
4.4: {"Amber": {"AmberCN": VGRemap({0: 0, 1: 1, 2: 2, 3: 3, 4: 4, 5: 5, 6: 6, 7: 7, 8: 8, 9: 9, 10: 10, 11: 11, 12: 12, 13: 13, 14: 14, 15: 15, 16: 16, 
                            17: 17, 18: 18, 19: 19, 20: 20, 21: 21, 22: 22, 23: 23, 24: 24, 25: 25, 26: 26, 27: 27, 28: 28, 29: 29, 30: 30, 31: 31, 
                            32: 32, 33: 33, 34: 34, 35: 35, 36: 36, 37: 37, 38: 38, 39: 39, 40: 40, 41: 41, 42: 42, 43: 43, 44: 44, 45: 45, 46: 46, 47: 47, 
                            48: 48, 49: 49, 50: 50, 51: 51, 52: 52, 53: 53, 54: 54, 55: 55, 56: 56, 57: 57, 58: 58, 59: 59, 60: 60, 61: 61, 62: 62, 63: 63, 64: 
                            64, 65: 65, 66: 66, 67: 67, 68: 68, 69: 69, 70: 70, 71: 71, 72: 72, 73: 73, 74: 74, 75: 75, 76: 76, 77: 77})},
      "AmberCN": {"Amber": VGRemap({0: 0, 1: 1, 2: 2, 3: 3, 4: 4, 5: 5, 6: 6, 7: 7, 8: 8, 9: 9, 10: 10, 11: 11, 12: 12, 13: 13, 14: 14, 15: 15, 16: 16, 
                            17: 17, 18: 18, 19: 19, 20: 20, 21: 21, 22: 22, 23: 23, 24: 24, 25: 25, 26: 26, 27: 27, 28: 28, 29: 29, 30: 30, 31: 31, 
                            32: 32, 33: 33, 34: 34, 35: 35, 36: 36, 37: 37, 38: 38, 39: 39, 40: 40, 41: 41, 42: 42, 43: 43, 44: 44, 45: 45, 46: 46, 47: 47, 
                            48: 48, 49: 49, 50: 50, 51: 51, 52: 52, 53: 53, 54: 54, 55: 55, 56: 56, 57: 57, 58: 58, 59: 59, 60: 60, 61: 61, 62: 62, 63: 63, 64: 
                            64, 65: 65, 66: 66, 67: 67, 68: 68, 69: 69, 70: 70, 71: 71, 72: 72, 73: 73, 74: 74, 75: 75, 76: 76, 77: 77})},
      "Mona": {"MonaCN": VGRemap({0: 0, 1: 1, 2: 2, 3: 3, 4: 4, 5: 5, 6: 6, 7: 7, 8: 8, 9: 9, 10: 10, 11: 11, 12: 12, 13: 13, 14: 14, 15: 15, 16: 16, 17: 17, 
                              18: 18, 19: 19, 20: 20, 21: 21, 22: 22, 23: 23, 24: 24, 25: 25, 26: 26, 27: 27, 28: 28, 29: 29, 30: 30, 31: 31, 32: 32, 33: 33, 
                              34: 34, 35: 35, 36: 36, 37: 37, 38: 38, 39: 39, 40: 40, 41: 41, 42: 42, 43: 43, 44: 44, 45: 45, 46: 46, 47: 47, 48: 48, 49: 49, 
                              50: 50, 51: 51, 52: 52, 53: 53, 54: 54, 55: 55, 56: 56, 57: 57, 58: 58, 59: 59, 60: 60, 61: 61, 62: 62, 63: 63, 64: 64, 65: 65, 
                              66: 66, 67: 67, 68: 68, 69: 69, 70: 70, 71: 71, 72: 72, 73: 73, 74: 74, 75: 75, 76: 76, 77: 77, 78: 78, 79: 79, 80: 80, 81: 81, 
                              82: 82, 83: 83, 84: 84, 85: 85, 86: 86, 87: 87, 88: 88, 89: 89, 90: 90, 91: 91, 92: 92, 93: 93, 94: 94, 95: 95, 96: 96, 97: 97, 
                              98: 98, 99: 99, 100: 100, 101: 101, 102: 102, 103: 103, 104: 104, 105: 105, 106: 106})},
      "MonaCN": {"Mona": VGRemap({0: 0, 1: 1, 2: 2, 3: 3, 4: 4, 5: 5, 6: 6, 7: 7, 8: 8, 9: 9, 10: 10, 11: 11, 12: 12, 13: 13, 14: 14, 15: 15, 16: 16, 17: 17, 
                              18: 18, 19: 19, 20: 20, 21: 21, 22: 22, 23: 23, 24: 24, 25: 25, 26: 26, 27: 27, 28: 28, 29: 29, 30: 30, 31: 31, 32: 32, 33: 33, 
                              34: 34, 35: 35, 36: 36, 37: 37, 38: 38, 39: 39, 40: 40, 41: 41, 42: 42, 43: 43, 44: 44, 45: 45, 46: 46, 47: 47, 48: 48, 49: 49, 
                              50: 50, 51: 51, 52: 52, 53: 53, 54: 54, 55: 55, 56: 56, 57: 57, 58: 58, 59: 59, 60: 60, 61: 61, 62: 62, 63: 63, 64: 64, 65: 65, 
                              66: 66, 67: 67, 68: 68, 69: 69, 70: 70, 71: 71, 72: 72, 73: 73, 74: 74, 75: 75, 76: 76, 77: 77, 78: 78, 79: 79, 80: 80, 81: 81, 
                              82: 82, 83: 83, 84: 84, 85: 85, 86: 86, 87: 87, 88: 88, 89: 89, 90: 90, 91: 91, 92: 92, 93: 93, 94: 94, 95: 95, 96: 96, 97: 97, 
                              98: 98, 99: 99, 100: 100, 101: 101, 102: 102, 103: 103, 104: 104, 105: 105, 106: 106})}}}
##### ENDLINK ############


# BossFixFormatter: Text formatting for the help page of the command 
class BossFixFormatter(argparse.MetavarTypeHelpFormatter, argparse.RawTextHelpFormatter):
    pass

# ConfigParserDict: Dictionary used to only keep the value of the first instance of a key
class ConfigParserDict(OrderedDict):
    def __setitem__(self, key, value):
        # All values updated into the dictionary of ConfigParser will first updated as a list of values, then
        #    the list of values will be turned into a string
        #
        # eg. the 'value' argument for the __setitem__ method in the case a key has 2 duplicates
        # >> value = ["val1"]           <----------- we only want this list
        # >> value = ["val1", "", "val2"]
        # >> value = ["val1", "", "val2", "", "val3"]
        # >> value = "val1\nval2\nval3"
        #
        # Note:
        #   For the case of duplicate keys, GIMI will only keep the value of the first valid instance of the key.
        #       Since checking for correct syntax and semantics is out of the scope of this program, we only get 
        #        the value of the first instance of the key
        if (key in self and isinstance(self[key], list) and isinstance(value, list)):
            return

        super().__setitem__(key, value)


# CommandBuilder: Class for building the command
class CommandBuilder():
    def __init__(self):
        self._argParser = argparse.ArgumentParser(description='Fixes Raiden Boss Phase 1 for all types of mods', formatter_class=BossFixFormatter)
        self._addArguments()

    def parseArgs(self) -> argparse.Namespace:
        return self._argParser.parse_args()

    def _addArguments(self):
        self._argParser.add_argument('-s', '--src', action='store', type=str, help="The starting path to run this fix. If this option is not specified, then will run the fix from the current directory.")
        self._argParser.add_argument('-v', VersionOpt, action='store', type=str, help="The game version we want the fix to be compatible with. If this option is not specified, then will use the latest game version")
        self._argParser.add_argument('-d', DeleteBackupOpt, action='store_true', help=f'deletes backup copies of the original {FileExt.Ini.value} files')
        self._argParser.add_argument('-f', FixOnlyOpt, action='store_true', help='only fixes the mod without cleaning any previous runs of the script')
        self._argParser.add_argument('-r', RevertOpt, action='store_true', help='reverts back previous runs of the script')
        self._argParser.add_argument('-l', '--log', action='store', type=str, help=f'The folder location to log the printed out text into a seperate {FileExt.Txt.value} file. If this option is not specified, then will not log the printed out text.')
        self._argParser.add_argument('-a', AllOpt, action='store_true', help=f'Parses all {FileTypes.Ini.value}s that the program encounters. This option supersedes the {TypeOpt} option')
        self._argParser.add_argument('-n', '--defaultType', action='store', type=str, help=f'''The default mod type to use if the {FileTypes.Ini.value} belongs to some unknown mod
        If the {AllOpt} is set to True, then this argument will be 'raiden'.
        Otherwise, if this value is not specified, then any mods with unknown types will be skipped

        See below for the different names/aliases of the supported types of mods.''')
        self._argParser.add_argument('-t', TypeOpt, action='store', type=str, help=f'''Parses {FileTypes.Ini.value}s that the program encounters for only specific types of mods. If the {AllOpt} option has been specified, this option has no effect. 
        By default, if this option is not specified, will parse the {FileTypes.Ini.value}s for all the supported types of mods. 

        Please specify the types of mods using the the mod type's name or alias, then seperate each name/alias with a comma(,)
        eg. raiden,arlecchino,ayaya

        See below for the different names/aliases of the supported types of mods.''')

    def addEpilog(self, epilog: str):
        self._argParser.epilog = epilog


class Error(Exception):
    """
    The base exception used by this module

    Parameters
    ----------
    message: :class:`str`
        the error message to print out
    """

    def __init__(self, message: str):
        super().__init__(f"ERROR: {message}")


class FileException(Error):
    """
    This Class inherits from :class:`Error`

    Exceptions relating to files

    Parameters
    ----------
    message: :class:`str`
        The error message to print out

    path: Optional[:class:`str`]
        The path where the error for the file occured. If this value is ``None``, then the path
        will be the current directory where this module is loaded :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, message: str, path: Optional[str] = None):
        path = FileService.getPath(path)

        if (path != DefaultPath):
            message += f" at {path}"

        super().__init__(message)


class DuplicateFileException(FileException):
    """
    This Class inherits from :class:`FileException`

    Exception when there are multiple files of the same type in a folder

    Parameters
    ----------
    files: List[:class:`str`]
        The files that triggered the exception

    fileType: :class:`str`
        The name for the type of files :raw-html:`<br />` :raw-html:`<br />`

        **Default**: "file"

    path: Optional[:class:`str`]
        The path to the folder where the files are located If this value is ``None``, then the path
        will be the current directory where this module is loaded :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    files: List[:class:`str`]
        The files that triggered the exception

    fileType: :class:`str`
        The name for the type of files

        **Default**: ``None``
    """

    def __init__(self, files: List[str], fileType: str = FileTypes.Default.value, path: Optional[str] = None):
        path = FileService.getPath(path)
        self.files = files
        self.fileType = fileType
        message = f"Ensure only one {fileType} exists"
        super().__init__(message, path = path)


class MissingFileException(FileException):
    """
    This Class inherits from :class:`FileException`

    Exception when a certain type of file is missing from a folder

    Parameters
    ----------
    fileType: :class:`str`
        The type of file searching in the folder :raw-html:`<br />` :raw-html:`<br />`

        **Default**: "file"

    path: :class:`str`
        The path to the folder that is being searched. If this value is ``None``, then the path
        will be the current directory where this module is loaded :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    fileType: :class:`str`
        The type of file searching in the folder
    """
    def __init__(self, fileType: str = FileTypes.Default.value, path: Optional[str] = None):
        path = FileService.getPath(path)
        message = f"Unable to find {fileType}. Ensure it is in the folder"
        self.fileType = fileType
        super().__init__(message, path = path)


class RemapMissingBlendFile(FileException):
    """
    This Class inherits from :class:`FileException`

    Exception when a RemapBlend.buf file is missing its corresponding Blend.buf file

    Parameters
    ----------
    remapBlend: :class:`str`
        The path to the RemapBlend.buf file
    """

    def __init__(self, remapBlend: str):
        super().__init__(f"Missing the corresponding Blend.buf file for the RemapBlend.buf", path = remapBlend)


class BlendFileNotRecognized(FileException):
    """
    This Class inherits from :class:`FileException`

    Exception when a Blend.buf file cannot be read

    Parameters
    ----------
    blendFile: :class:`str`
        The file path to the Blend.buf file
    """
    def __init__(self, blendFile: str):
        super().__init__(f"Blend file format not recognized for {os.path.basename(blendFile)}", path = os.path.dirname(blendFile))

class BadBlendData(Error):
    """
    This Class inherits from :class:`Error`

    Exception when certain bytes do not correspond to the format defined for a Blend.buf file
    """

    def __init__(self):
        super().__init__(f"Bytes do not corresponding to the defined format for a Blend.buf file")


class ConflictingOptions(Error):
    """
    This Class inherits from :class:`Error`

    Exception when the script or :class:`RemapService` is ran with options that cannot be used together

    Parameters
    ----------
    options: List[:class:`str`]
        The options that cannot be used together
    """
    def __init__(self, options: List[str]):
        optionsStr = ", ".join(options)
        super().__init__(f"The following options cannot be used toghether: {optionsStr}")

class InvalidModType(Error):
    """
    This Class inherits from :class:`Error`

    Exception when the type of mod specified to fix is not found

    Parameters
    ----------
    type: :class:`str`
        The name for the type of mod specified
    """
    def __init__(self, type: str):
        super().__init__(f"Unable to find the type of mod by the search string, '{type}'")

class NoModType(Error):
    """
    This Class inherits from :class:`Error`

    Exception when trying to fix a mod of some unidentified mod type

    Parameters
    ----------
    type: :class:`str`
        The name for the type of mod specified 
    """

    def __init__(self):
        super().__init__(f"No mod type specified when fixing the .ini file")


class ListTools():
    """
    Tools for handling with Lists
    """

    @classmethod
    def getDistinct(cls, lst: List[Any], keepOrder: bool = False) -> List[Any]:
        """
        Makes all the elements in the list unique

        Parameters
        ----------
        lst: List[Any]
            The list we are working with

        keepOrder: bool
            Whehter to keep the order of the elements in the list :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        List[Any]
            The new list with only unique values
        """

        if (keepOrder):
            return list(OrderedDict.fromkeys(lst))
        return list(set(lst))
    

    @classmethod
    def removeParts(cls, lst: List[T], partIndices: List[Tuple[int, int]], nullifyRemoval: Callable[[], N], isNull: Callable[[Union[T, N]], bool]) -> List[T]:
        """
        Removes many sub-lists from a list

        Parameters
        ----------
        lst: List[T]
            The desired list to have its parts removed

        partIndices: List[Tuple[:class:`int`, :class:`int`]]:
            The indices relating to the parts to be removed from the lists :raw-html:`<br />` :raw-html:`<br />`

            The tuples contain:

                #. The starting index of the part
                #. The ending index of the part (excluded from the actual list)

        nullifyRemoval: Callable[[], N]:
            Function for creating a null element used to replace the removed part

        isNull: Callable[[Union[T, N]], :class:`bool`]
            Function for identifying whether an element in the list is the null element

        Returns
        -------
        List[T]
            The new list with its parts removed
        """

        null = nullifyRemoval()
        for indices in partIndices:
            startInd = indices[0]
            endInd = indices[1]
            lst[startInd:endInd] =  [null] * (endInd - startInd)

        lst = list(filter(lambda element: not isNull(element), lst))
        return lst

class DictTools():
    """
    Tools for handling with Dictionaries
    """

    @classmethod
    def getFirstKey(cls, dict: Dict[Any, Any]) -> Any:
        """
        Retrieves the first key in a dictionary

        Parameters
        ----------
        dict: Dict[Any, Any]
            The dictionary we are working with

            .. note::
                The dictionary must not be empty

        Returns
        -------
        Any
            The first key of the dictionary
        """

        return next(iter(dict))

    @classmethod
    def getFirstValue(cls, dict: Dict[Any, Any]) -> Any:
        """
        Retrieves the first value in a dictionary

        Parameters
        ----------
        dict: Dict[Any, Any]
            The dictionary we are working with

        Returns
        -------
        Any
            The first value of the dictionary
        """

        return dict[cls.getFirstKey(dict)]
    
    @classmethod
    def update(cls, srcDict: Dict[Hashable, Any], newDict: Dict[Hashable, Any], combineDuplicate: Optional[Callable[[Any, Any], Any]] = None) -> Dict[Hashable, Any]:
        """
        Updates ``srcDict`` based off the new values from ``newDict``

        Parameters
        ----------
        srcDict: Dict[Hashable, Any]
            The dictionary to be updated

        newDict: Dict[Hashable, Any]
            The dictionary to help with updating ``srcDict``

        combineDuplicate: Optional[Callable[[Any, Any], Any]]
            Function for handling cases where there contains the same key in both dictionaries :raw-html:`<br />` :raw-html:`<br />`

            * The first parameter comes from ``srcDict``
            * The second parameter comes from ``newDict``

            If this value is set to ``None``, then will use the key from ``newDict`` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Dict[Hashable, Any]
            Reference to the updated dictionary
        """

        if (combineDuplicate is None):
            srcDict.update(newDict)
            return srcDict
        
        combinedValues = {}
        srcDictLen = len(srcDict)
        newDictLen = len(newDict)
        
        shortDict = srcDict
        longDict = newDict
        if (srcDictLen > newDictLen):
            shortDict = newDict
            longDict = srcDict

        for key in shortDict:
            if (key in longDict):
                combinedValues[key] = combineDuplicate(srcDict[key], newDict[key])

        srcDict.update(newDict)
        srcDict.update(combinedValues)
        return srcDict


    @classmethod
    def combine(cls, dict1: Dict[Hashable, Any], dict2: Dict[Hashable, Any], combineDuplicate: Optional[Callable[[Any, Any], Any]] = None) -> Dict[Hashable, Any]:
        """
        Creates a new dictionary from combining 2 dictionaries

        Parameters
        ----------
        dict1: Dict[Hashable, Any]
            The destination of where we want the combined dictionaries to be stored

        dict2: Dict[Hashable, Any]
            The dictionary we want to combine with

        combineDuplicate: Optional[Callable[[Any, Any], Any]]
            Function for handling cases where there contains the same key in both dictionaries

            If this value is set to ``None``, then will use the key from 'dict2' :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        makeNewCopy: :class:`bool`
            Whether we want the resultant dictionary to be newly created or to be updated into ``dict1``

        Returns
        -------
        Dict[Hashable, Any]
            The new combined dictionary
        """

        new_dict = {**dict1, **dict2}

        if (combineDuplicate is None):
            return new_dict

        for key in new_dict:
            if key in dict1 and key in dict2:
                new_dict[key] = combineDuplicate(new_dict[key], dict1[key])

        return new_dict
    
    @classmethod
    def invert(cls, dict: Dict[Hashable, Hashable]) -> Dict[Hashable, Hashable]:
        """
        Inverts a dictionary by making the keys the values and the values the keys

        Parameters
        ----------
        dict: Dict[Hashable, Hashable]
            The dictionary to invert

        Returns
        -------
        Dict[Hashable, Hashable]
            The inverted dictionary
        """

        return {v: k for k, v in dict.items()}
    

class TextTools():
    @classmethod
    def removeParts(cls, txt: str, partIndices: List[Tuple[int, int]]) -> str:
        """
        Remove multiple substrings from a text based off the indices of the substrings

        Parameters
        ----------
        txt: :class:`str`
            The target txt to have the substrings removed

        partIndices: List[Tuple[:class:`int`, :class:`int`]]
            The indices for the substrings to be removed :raw-html:`<br />` :raw-html:`<br />`

            The tuples contain the following data:

                #. The start index for the substring
                #. The ending index for the substring

        Returns 
        -------
        :class:`str`
            The new string with the substrings removed
        """

        chars = list(txt)
        chars = ListTools.removeParts(chars, partIndices, lambda: 0, lambda element: element == 0)
        result = "".join(chars)
        return result


    @classmethod
    def removeLines(cls, txtLines: List[str], partIndices: List[Tuple[int, int]]) -> List[str]:
        """
        Removes multiple sub-lists of lines from a list of text lines

        Parameters
        ----------
        txtLines: List[:class:`str`]
            The lines of text to have its lines removed

        partIndices: List[Tuple[:class:`int`, :class:`int`]]
            The indices for the list of lines to be removed :raw-html:`<br />` :raw-html:`<br />`

            The tuples contain the following data:

                #. The start index for the list of lines
                #. The ending index for the list of lines

        Returns 
        -------
        List[:class:`str`]
            The new lines of text with the removed lines
        """

        result = ListTools.removeParts(txtLines, partIndices, lambda: 0, lambda element: element == 0)
        return result
    
    @classmethod
    def getTextLines(cls, txt: str) -> List[str]:
        """
        Retrieves the lines of text, split by the newline character, similar to how python's `readlines`_ function works

        Parameters
        ----------
        txt: :class:`str`
            The target text to be split

        Returns
        -------
        List[:class:`str`]
            The lines of text that were split
        """

        txtLines = txt.split("\n")

        if (txt):
            txtLinesLen = len(txtLines)
            for i in range(txtLinesLen):
                if (i < txtLinesLen - 1):
                    txtLines[i] += "\n"
        else:
            txtLines = []

        return txtLines


class Cache(Generic[T]):
    """
    Class for a generic cache

    .. container:: operations

        **Supported Operations:**

        .. describe:: len(x)

            Retrieves the size of the :class:`Cache`, ``x``

        .. describe:: x[key]

            Retrieves the value from the :class:`Cache`, ``x``, from the key ``key``

        .. describe:: x[key] = newValue

            Sets the key ``key`` of the :class:`Cache`, ``x``, to have the value of ``newValue``

    :raw-html:`<br />`

    Parameters
    ----------
    capacity: :class:`int`
        The maximum capacity of the cache :raw-html:`<br />` :raw-html:`<br />`

        **Default**: 128

    cacheStorage: Optional[Any]
        The type of `KVP`_ (Key-value pair) data structure to use for the cache. If this parameter is ``None``, then will use a dictionary :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    capacity: :class:`int`
        The maximum capacity of the cache

    cacheStorage: Any
        The type of `KVP`_ (Key-value pair) data structure to use for the cache.
    """

    def __init__(self, capacity: int = DefaultCacheSize, cacheStorage: Optional[Any] = None):
        self.capacity = capacity

        if (cacheStorage is None):
            self._cache = {}
        else:
            self._cache = cacheStorage

    def __getitem__(self, key: Hashable) -> Optional[T]:
        return self._cache[key]

    def __setitem__(self, key: Hashable, value: T) -> None:
        self._cache[key] = value

    def __len__(self) -> int:
        return len(self._cache)

    def clear(self) -> None:
        """
        Clears the cache
        """
        self._cache.clear()


class LruCache(Cache):
    """
    This class inherits from :class:`Cache`

    Class for an `LRU cache`_

    .. container:: operations

        **Supported Operations:**

        .. describe:: len(x)

            Retrieves the size of the :class:`LruCache`, ``x``

        .. describe:: x[key]

            Retrieves the value from the :class:`LruCache`, ``x``, from the key ``key``

        .. describe:: x[key] = newValue

            Sets the key ``key`` of the :class:`LruCache`, ``x``, to have the value of ``newValue``

    :raw-html:`<br />`

    Parameters
    ----------
    capacity: :class:`int`
        The maximum capacity of the cache :raw-html:`<br />` :raw-html:`<br />`

        **Default**: 128
    """

    def __init__(self, capacity: int = DefaultCacheSize):
        super().__init__(capacity, OrderedDict())

    def __getitem__(self, key: Hashable) -> Optional[T]:
        if key not in self._cache:
            raise KeyError(f"{key}")

        self._cache.move_to_end(key)
        return self._cache[key]

    def __setitem__(self, key: Hashable, value: T) -> None:
        if len(self._cache) == self.capacity:
            self._cache.popitem(last=False)

        self._cache[key] = value
        self._cache.move_to_end(key)


class Algo():
    """
    Tools for some basic algorithms
    """

    @classmethod
    def _getMid(cls, left, right) -> int:
        return int(left + (right - left) / 2)

    @classmethod
    def binarySearch(cls, lst: List[T], target: T, compare: Callable[[T, T], bool]) -> List[Union[int, bool]]:
        """
        Performs `binary search`_ to search for 'target' in 'lst'

        Parameters
        ----------
        lst: List[T]
            The sorted list we are searching from

        target: T
            The target element to search for in the list

        compare: Callable[[T, T], :class:`bool`]
            The `compare function`_ for comparing elements in the list with the target element

        Returns
        -------
        [:class:`int`, :class:`bool`]
            * The first element is whether the target element is found in the list
            * The second element is the found index or the index that we expect the target element to be in the list
        """

        left = 0
        right = len(lst) - 1
        mid = cls._getMid(left, right)

        while (left <= right):
            midItem = lst[mid]
            compResult = compare(midItem, target)

            if (compResult == 0):
                return [True, mid]
            elif (compResult > 0):
                right = mid - 1
            else:
                left = mid + 1

            mid = cls._getMid(left, right)

        return [False, left]
    
    @classmethod
    def binaryInsert(cls, lst: List[T], target: T, compare: Callable[[T, T], bool], optionalInsert: bool = False) -> bool:
        """
        Insert's 'target' into 'lst' using `binary search`_

        Parameters
        ----------
        lst: List[T]
            The sorted list we want to insert the target element

        target: T
            The target element to insert

        compare: Callable[[T, T], :class:`bool`]
            The `compare function`_ for comparing elements in the list with the target element

        optionalInsert: :class:`bool`
            Whether to still insert the target element into the list if the element target element is found in the list :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`bool`
            Whether the target element has been inserted into the list
        """

        found = False
        inserted = False

        found, insertInd = cls.binarySearch(lst, target, compare)
        if (not optionalInsert or not found):
            lst.insert(insertInd, target)
            inserted = True

        return inserted


class FileService():
    """
    Tools for handling with files and folders :raw-html:`<br />` :raw-html:`<br />`
    """

    @classmethod
    def getFilesAndDirs(cls, path: Optional[str] = None, recursive: bool = False) -> List[List[str]]:
        """
        Retrieves the files and folders contained in a certain folder

        Parameters
        ----------
        path: Optional[:class:`str`]
            The path to the target folder we are working with. If this argument is ``None``, then will use the current directory of where this module is loaded
            :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        recursive: :class:`bool`
            Whether to recursively check all the folders from our target folder :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        [List[:class:`str`], List[:class:`str`]]
            The files and directories within the folder. The order for the result is:

            #. files
            #. folders
        """
        path = cls.getPath(path)
        files = []
        dirs = []

        pathItems = []
        
        if (recursive):
            for root, currentDirs, currentFiles in os.walk(path, topdown = True):
                for dir in currentDirs:
                    dirs.append(os.path.join(root, dir))

                for file in currentFiles:
                    files.append(os.path.join(root, file))

            return [files, dirs]
        
        pathItems = os.listdir(path)
        for itemPath in pathItems:
            fullPath = os.path.join(path, itemPath)
            if (os.path.isfile(fullPath)):
                files.append(fullPath)
            else:
                dirs.append(fullPath)

        return [files, dirs]

    # filters and partitions the files based on the different filters specified
    @classmethod
    def getFiles(cls, path: Optional[str] = None, filters: Optional[List[Callable[[str], bool]]] = None, files: Optional[List[str]] = None) -> Union[List[str], List[List[str]]]:
        """
        Retrieves many different types of files within a folder

        .. note::
            Only retrieves files that are the direct children of the folder (will not retrieve files nested in a folder within the folder we are searching)

        Parameters
        ----------
        path: Optional[:class:`str`]
            The path to the target folder we are working with. If this value is set to ``None``, then will use the current directory of where this module is loaded
            :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        filters: Optional[List[Callable[[:class:`str`], :class:`bool`]]]
            Different filter functions for each type of file we are trying to get. If this values is either ``None`` or ``[]``, then will default to a filter to get all the files :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        files: Optional[List[:class:`str`]]
            The files contained in the target folder

            If this value is set to ``None``, then the function will search for the files :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Union[List[:class:`str`], List[List[:class:`str`]]]
            The files partitioned into the different types specified by the filters

            If 'filters' only has 1 element, then the function returns List[:class:`str`]
            Otherwise, will return List[List[:class:`str`]]
        """

        path = cls.getPath(path)
        result = []

        if (filters is None):
            filters = []

        if (not filters):
            filters.append(lambda itemPath: True)

        filtersLen = len(filters)
        usePathFiles = False
        if (files is None):
            files = os.listdir(path)
            usePathFiles = True

        for i in range(filtersLen):
            result.append([])
        
        for itemPath in files:
            for filterInd in range(filtersLen):
                pathFilter = filters[filterInd]
                if (not pathFilter(itemPath) or (usePathFiles and not os.path.isfile(os.path.join(path, itemPath)))):
                    continue

                fullPath = os.path.join(path, itemPath)

                result[filterInd].append(fullPath)

        if (filtersLen == 1):
            return result[0]
        
        return result
    
    # retrieves only a single file for each filetype specified by the filters
    @classmethod
    def getSingleFiles(cls, path: Optional[str] = None, filters: Optional[Dict[str, Callable[[str], bool]]] = None, files: Optional[List[str]] = None, optional: bool = False) -> Union[Optional[str], List[str], List[Optional[str]]]:
        """
        Retrieves exactly 1 of each type of file in a folder

        Parameters
        ----------
        path: Optional[:class:`str`]
            The path to the target folder we are searching. :raw-html:`<br />` :raw-html:`<br />`
            
            If this value is set to ``None``, then will use the current directory of where this module is loaded :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        filters: Optional[Dict[str, Callable[[:class:`str`], :class:`bool`]]]
            Different filter functions for each type of file we are trying to get. If this value is ``None`` or ``{}``, then will default to use a filter to get all files

            The keys are the names for the file type :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        files: Optional[List[:class:`str`]]
            The files contained in the target folder

            If this value is set to ``None``, then the function will search for the files :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        optional: :class:`bool`
            Whether we want to send an exception if there is not exactly 1 file for a certain type of file :raw-html:`<br />` :raw-html:`<br />`

            #. If this value is ``False`` and there are no files for a certain type of file, then will raise a :class:`MissingFileException`
            #. If this value is ``False`` and there are more than 1 file for a certain type of file, then will raise a :class:`DuplicateFileException`
            #. If this value is ``True`` and there are no files for a certain type of file, then the file for that type of file will be ``None``
            #. If this value is ``True`` and there are more than 1 file for a certain type of file, then will retrieve the first file for that type of file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Raises
        ------
        :class:`MissingFileException`
            if ``optional`` is set to ``False`` and there are not files for a certain type of file

        :class:`DuplicateFileException`
            if ``optional`` is set to ``False`` and there are more than 1 file for a certain type of file

        Returns
        -------
        Union[Optional[:class:`str`], List[:class:`str`], List[Optional[:class:`str`]]]
            The files partitioned for each type of file

            * If ``filters`` only contains 1 element and ``optional`` is ``False``, then will return :class:`str`
            * If ``filters`` contains more than 1 element and ``optional`` is ``False`, then will return List[:class:`str`]
            * If ``filters`` only contains 1 element and ``optional`` is ``True``, then will return Optional[:class:`str`]
            * Otherwise, returns List[Optional[:class:`str`]]
        """
        path = cls.getPath(path)
        if (filters is None):
            filters = {}

        if (not filters):
            filters[FileTypes.Default.value] = lambda itemPath: True
        
        filesPerFileTypes = cls.getFiles(path = path, filters = list(filters.values()), files = files)
        filtersLen = len(filters)

        onlyOneFilter = filtersLen == 1
        if (onlyOneFilter):
            filesPerFileTypes = [filesPerFileTypes]

        result = []
        i = 0
        for fileType in filters:
            fileTypeFiles = filesPerFileTypes[i]
            filesLen = len(fileTypeFiles)

            if (not optional and not filesLen):
                raise MissingFileException(fileType = fileType, path = path)
            elif (not optional and filesLen > 1):
                raise DuplicateFileException(fileTypeFiles, fileType = fileType, path = path)
            
            if (fileTypeFiles):
                result.append(fileTypeFiles[0])
            else:
                result.append(None)
            i += 1

        if (onlyOneFilter):
            return result[0]
        
        return result
    
    @classmethod
    def rename(cls, oldFile: str, newFile: str):
        """
        Renames a file

        .. warning::
            If the new name for the file already exists, then the function deletes
            the file with the new name and renames the target file with the new name

        Parameters
        ----------
        oldFile: :class:`str`
            file path to the target file we are working with

        newFile: :class:`str`
            new file path for the target file 
        """
        if (oldFile == newFile):
            return

        try:
            os.rename(oldFile, newFile)
        except FileExistsError:
            os.remove(newFile)
            os.rename(oldFile, newFile)

    @classmethod
    def changeExt(cls, file: str, newExt: str) -> str:
        """
        Changes the extension for a file

        Parameters
        ----------
        file: :class:`str`
            The file path to the file we are working with

        newExt: :class:`str`
            The name of the new extension for the file (without the dot at front)

        Returns
        -------
        :class:`str`
            the new file path with the extension changed
        """

        dotPos = file.rfind(".")

        if (not newExt.startswith(".")):
            newExt = f".{newExt}"

        if (dotPos != -1):
            file = file[:dotPos] + newExt

        return file

    @classmethod
    def disableFile(cls, file: str, filePrefix: str = BackupFilePrefix) -> str:
        """
        Marks a file as 'DISABLED' and changes the file to a .txt file

        Parameters
        ----------
        file: :class:`str`
            The file path to the file we are working with

        filePrefix: :class:`str`
            Prefix name we want to add in front of the file name :raw-html:`<br />` :raw-html:`<br />`

            **Default**: "DISABLED_BossFixBackup\_"

        Returns
        -------
        :class:`str`
            The new name of the file
        """

        baseName = os.path.basename(file)
        baseName = FileService.changeExt(baseName, FileExt.Txt.value)

        backupFile = os.path.join(os.path.dirname(file), filePrefix + baseName)
        FileService.rename(file, backupFile)
        return backupFile

    @classmethod
    def copyFile(cls, src: str, dest: str):
        """
        Copies a file from ``src`` to ``dest``

        Parameters
        ----------
        src: :class:`str`
            The file path to the file to be copied

        dest: :class:`str`
            The new file path for the copied file
        """

        shutil.copy2(src, dest)

    @classmethod
    def parseOSPath(cls, path: str):
        """
        Retrieves a normalized file path from a string

        Parameters
        ----------
        path: :class:`str`
            The string containing some sort of file path
        """

        result = ntpath.normpath(path)
        result = cls.ntPathToPosix(result)
        return result

    @classmethod
    def ntPathToPosix(cls, path: str) -> str:
        """
        Converts a file path from the `ntpath <https://opensource.apple.com/source/python/python-3/python/Lib/ntpath.py.auto.html>`_ library to a file path for the `os <https://docs.python.org/3/library/os.html>`_ library

        .. note::
            The character for the folder paths (``/`` or ``\\``) used in both libraries may be different depending on the OS

        Parameters
        ----------
        path: :class:`str`
            The file path we are working that is generated from the 'ntpath' library

        Returns
        -------
        :class:`str`
            The file path generated by the 'os' library
        """

        return path.replace(ntpath.sep, os.sep)
    
    @classmethod
    def absPathOfRelPath(cls, dstPath: str, relFolder: str) -> str:
        """
        Retrieves the absolute path of the relative path of a file with respect to a certain folder

        Parameters
        ----------
        dstPath: :class:`str`
            The target file path we are working with

        relFolder: :class:`str`
            The folder that the target file path is relative to

        Returns
        -------
        :class:`str`
            The absolute path for the target file
        """

        relFolder = os.path.abspath(relFolder)
        result = dstPath
        if (not os.path.isabs(result)):
            result = os.path.join(relFolder, result)

        return cls.parseOSPath(result)
    
    @classmethod
    def getRelPath(cls, path: str, start: str) -> str:
        """
        Tries to get the relative path of a file/folder relative to another folder, if possible.

        If it is not possible to get the relative path, will return back the original file path

        .. note::
            An example where it would not be possible to get the relative path would be:
            
            * If the file is located in one mount (eg. C:/ drive) and the folder is located in another mount (eg. D:/ drive)

        Parameters
        ----------
        path: :class:`str`
            The path to the target file/folder we are working with

        start: :class:`str`
            The path that the target file/folder is relative to

        Returns
        -------
        :class:`str`
            Either the relative path or the original path if not possible to get the relative paths
        """

        result = path
        try:
            result = os.path.relpath(path, start)

        # if the path is in another mount than 'start'
        except ValueError:
            pass

        return cls.parseOSPath(result)
    
    # read(file, fileCode, postProcessor): Tries to read a file using different encodings
    @classmethod
    def read(cls, file: str, fileCode: str, postProcessor: Callable[[TextIoWrapper], Any]) -> Any:
        """
        Tries to read a file using different file encodings

        Will interact with the file using the following order of encodings:

        #. utf-8 
        #. latin1

        Parameters
        ----------
        file: :class:`str`
            The file we are trying to read from

        fileCode: :class:`str`
            What `file mode <https://docs.python.org/3/tutorial/inputoutput.html#reading-and-writing-files>`_ to interact with the file (eg. r, rb, r+, etc...)

        postProcessor: Callable[[`TextIoWrapper`_], Any]
            A function used to process the file pointer of the opened file

        Returns
        -------
        Any
            The result after processing the file pointer of the opened file
        """

        error = None
        for encoding in ReadEncodings:
            try:
                with open(file, fileCode, encoding = encoding) as f:
                    return postProcessor(f)
            except UnicodeDecodeError as e:
                error = e

        if (error is not None):
            raise UnicodeDecodeError(f"Cannot decode the file using any of the following encodings: {ReadEncodings}")
        
    @classmethod
    def readBinary(cls, src: Union[str, bytes]) -> bytes:
        """
        Reads a binary file

        Parameters
        ----------
        src: Union[:class:`str`, :class:`bytes`]
            The source to read from

        Returns
        -------
        :class:`bytes`
            The read bytes
        """

        result = None
        if (isinstance(src, str)):
            with open(src, "rb") as f:
                result = f.read()
        else:
            result = src

        return result
    
    @classmethod
    def writeBinary(cls, file: str, data: bytes):
        """
        Writes data into a binary file

        Parameters
        ----------
        file: :class:`str`
            The file to write into

        data: :class:`bytes`
            The data to write
        """

        with open(file, "wb") as f:
            f.write(data)

    @classmethod
    def getPath(cls, path: Optional[str]) -> str:
        if (path is None):
            return DefaultPath
        return path
    

class FilePath():
    """
    Class for storing info about a file path

    Parameters
    ----------
    path: :class:`str`
        The file path
    """

    def __init__(self, path: str):
        self._folder = ""
        self._base = ""
        self.path = path

    @property
    def path(self):
        """
        The file path

        :getter: Retrieves the path
        :setter: Sets a new path
        :type: :class:`str`
        """
        return self._path
    
    @path.setter
    def path(self, newPath: str):
        self._path = newPath
        self._folder = os.path.dirname(newPath)
        self._base = os.path.basename(newPath)

    @property
    def folder(self):
        """
        The parent folder for the path

        :getter: Retrieves the parent folder name
        :type: :class:`str`
        """
        return self._folder
    
    @property
    def base(self):
        """
        The basename for the file path

        :getter: Retrieves the basename
        :type: :class:`str`
        """
        return self._base


class Heading():
    """
    Class for handling information about a heading for pretty printing

    Examples
    --------

    .. code-block:: python
        :linenos:
        :emphasize-lines: 1,3

        ======= Title: Fix Raiden Boss 2 =======
        ...
        ========================================

    Parameters
    ----------
    title: :class:`str`
        The title for the heading :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ""

    sideLen: :class:`int`
        The number of characters we want one side for the border of the opening heading to have :raw-html:`<br />` :raw-html:`<br />`

        **Default**: 0

    sideChar: :class:`str`
        The type of character we want the border for the heading to have  :raw-html:`<br />` :raw-html:`<br />`

        **Default**: "="

    Attributes
    ----------
    title: :class:`str`
        The title for the heading

    sideLen: :class:`int`
        The number of characters we want one side for the border of the opening heading to have

    sideChar: :class:`str`
        The type of character we want the border for the heading to have
    """

    def __init__(self, title: str = "", sideLen: int = 0, sideChar: str = "="):
        self.title = title
        self.sideLen = sideLen
        self.sideChar = sideChar

    def copy(self):
        """
        Makes a new copy of a heading

        Returns
        -------
        :class:`Heading`
            The new copy of the heading
        """
        return Heading(title = self.title, sideLen = self.sideLen, sideChar = self.sideChar)

    def open(self) -> str:
        """
        Makes the opening heading (see line 1 of the example at :class:`Heading`)

        Returns
        -------
        :class:`str`
            The opening heading created
        """

        side = self.sideLen * self.sideChar
        return f"{side} {self.title} {side}"

    def close(self) -> str:
        """
        Makes the closing heading (see line 3 of the example at :class:`Heading`)

        Returns
        -------
        :class:`str`
            The closing heading created
        """

        return self.sideChar * (2 * (self.sideLen + 1) + len(self.title))


class Logger():
    """
    Class for pretty printing output to display on the console

    Parameters
    ----------
    prefix: :class:`str`
        line that is printed before any message is printed out :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ""

    logTxt: :class:`bool`
        Whether to log all the printed messages into a .txt file once the fix is done :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    verbose: :class:`bool`
        Whether to print out output :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    Attributes
    ----------
    includePrefix: :class:`bool`
        Whether to include the prefix string when printing out a message

    verbose: :class:`bool`
        Whether to print out output

    logTxt: :class:`bool`
        Whether to log all the printed messages into a .txt file once the fix is done

    _prefix: :class:`str`
        line that is printed before any message is printed out

    _headings: Deque[:class:`Heading`]
        A stack of headings that have been opened (by calling :meth:`Heading.open`), but have not been closed yet (have not called :meth:`Heading.close` yet)

    _loggedTxt: :class:`str`
        The text that will be logged into a .txt file
    """

    DefaultHeadingSideLen = 2
    DefaultHeadingChar = "="

    def __init__(self, prefix: str = "", logTxt: bool = False, verbose: bool = True):
        self._prefix = prefix
        self.includePrefix = True
        self.verbose = verbose
        self.logTxt = logTxt
        self._loggedTxt = ""
        self._headings = deque()
        self._currentPrefixTxt = ""

        self._setDefaultHeadingAtts()

    @property
    def prefix(self):
        """
        The line of text that is printed before any message is printed out

        :getter: Returns such a prefix
        :setter: Sets up such a prefix for the logger
        :type: :class:`str`
        """
        return self._prefix
    
    @prefix.setter
    def prefix(self, newPrefix):
        self._prefix = newPrefix
        self._currentPrefixTxt = ""

    @property
    def loggedTxt(self):
        """
        The text to be logged into a .txt file

        :getter: Returns such a prefix
        :type: :class:`str`
        """
        return self._loggedTxt

    def clear(self):
        """
        Clears out any saved text from the logger
        """

        self._loggedTxt = ""

    def _setDefaultHeadingAtts(self):
        """
        Sets the default attributes for printing out a header line
        """

        self._headingTxtLen = 0
        self._headingSideLen = self.DefaultHeadingSideLen
        self._headingChar = self.DefaultHeadingChar

    def _addLogTxt(self, txt: str):
        """
        Appends the text to the logged output to be printed to a .txt file

        Parameters
        ----------
        txt: :class:`str`
            The text to be added onto the logged output
        """

        if (self.logTxt):
            self._loggedTxt += f"{txt}\n"

    def getStr(self, message: str):
        """
        Retrieves the string to be printed out by the logger

        Parameters
        ----------
        message: :class:`str`
            The message we want to print out

        Returns
        -------
        :class:`str`
            The transformed text that the logger prints out
        """

        return f"# {self._prefix} --> {message}"

    def log(self, message: str):
        """
        Regularly prints text onto the console

        Parameters
        ----------
        message: :class:`str`
            The message we want to print out
        """

        if (self.includePrefix):
            message = self.getStr(message)

        self._addLogTxt(message)
        self._currentPrefixTxt += f"{message}\n"

        if (self.verbose):
            print(message)

    def split(self):
        """
        Prints out a new line
        """

        if (self._currentPrefixTxt):
            self.log("\n")

    def space(self):
        """
        Prints out a space
        """
        self.log("")

    def openHeading(self, txt: str, sideLen: int = DefaultHeadingSideLen, headingChar = DefaultHeadingChar):
        """
        Prints out an opening heading

        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out

        sideLen: :class:`int`
            How many characters we want for the side border of the heading :raw-html:`<br />`
            (see line 1 of the example at :class:`Heading`) :raw-html:`<br />` :raw-html:`<br />`

            **Default**: 2

        headingChar: :class:`str`
            The type of character used to print the side border of the heading :raw-html:`<br />`
            (see line 3 of the example at :class:`Heading`) :raw-html:`<br />` :raw-html:`<br />`

            **Default**: "="
        """

        heading = Heading(title = txt, sideLen = sideLen, sideChar = headingChar)
        self._headings.append(heading)
        self.log(heading.open())

    def closeHeading(self):
        """
        Prints out a closing heading that corresponds to a previous opening heading printed (see line 3 of the example at :class:`Heading`)
        """

        if (not self._headings):
            return

        heading = self._headings.pop()
        self.log(heading.close())

    @classmethod
    def getBulletStr(self, txt: str) -> str:
        """
        Creates the string for an item in an unordered list

        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out

        Returns
        -------
        :class:`str`
            The text formatted as an item in an unordered list
        """
        return f"- {txt}"
    
    @classmethod
    def getNumberedStr(self, txt: str, num: int) -> str:
        """
        Creates the string for an ordered list

        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out

        num: :class:`str`
            The number we want to print out before the text for the ordered list

        Returns
        -------
        :class:`str`
            The text formatted as an item in an ordered list
        """
        return f"{num}. {txt}"

    def bulletPoint(self, txt: str):
        """
        Prints out an item in an unordered list

        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out
        """
        self.log(self.getBulletStr(txt))

    def list(self, lst: List[str], transform: Optional[Callable[[str], str]] = None):
        """
        Prints out an ordered list

        Parameters
        ----------
        lst: List[:class:`str`]
            The list of messages we want to print out

        transform: Optional[Callable[[:class:`str`], :class:`str`]]
            A function used to do any processing on each message in the list of messages

            If this parameter is ``None``, then the list of message will not go through any type of processing :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        if (transform is None):
            transform = lambda txt: txt

        lstLen = len(lst)
        for i in range(lstLen):
            newTxt = transform(lst[i])
            self.log(self.getNumberedStr(newTxt, i + 1))

    def box(self, message: str, header: str):
        """
        Prints the message to be sandwiched by the text defined in the argument, ``header``

        Parameters
        ----------
        message: :class:`str`
            The message we want to print out

        header: :class:`str`
            The string that we want to sandwich our message against
        """

        self.log(header)

        messageList = message.split("\n")
        for messagePart in messageList:
            self.log(messagePart)

        self.log(header)

    def error(self, message: str):
        """
        Prints an error message

        Parameters
        ----------
        message: :class:`str`
            The message we want to print out
        """

        prevVerbose = self.verbose
        if (not self.logTxt):
            self.verbose = True

        self.space()

        self.box(message, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
        self.space()
        self.verbose = prevVerbose

    def handleException(self, exception: Exception):
        """
        Prints the message for an error

        Parameters
        ----------
        exception: :class:`Exception`
            The error we want to handle
        """

        message = f"\n{type(exception).__name__}: {exception}\n\n{traceback.format_exc()}"
        self.error(message)

    def input(self, desc: str) -> str:
        """
        Handles user input from the console

        Parameters
        ----------
        desc: :class:`str`
            The question/description being asked to the user for input

        Returns
        -------
        :class:`str`
            The resultant input the user entered
        """

        if (self.includePrefix):
            desc = self.getStr(desc)

        self._addLogTxt(desc)
        result = input(desc)
        self._addLogTxt(f"Input: {result}")

        return result

    def waitExit(self):
        """
        Prints the message used when the script finishes running
        """

        prevIncludePrefix = self.includePrefix
        self.includePrefix = False
        self.input("\n== Press ENTER to exit ==")
        self.includePrefix = prevIncludePrefix 


# our model objects in MVC
class Model():
    """
    Generic class used for any data models in the fix

    Parameters
    ----------
    logger: Optional[:class:`Logger`]
        The logger used to print messages to the console :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    logger: Optional[:class:`Logger`]
        The logger used to print messages to the console
    """
    def __init__(self, logger: Optional[Logger] = None):
        self.logger = logger

    def print(self, funcName: str, *args, **kwargs):
        """
        Prints out output

        Parameters
        ----------
        funcName: :class:`str`
            The name of the function in the logger for printing out the output

        \*args: List[:class:`str`]
            Arguments to pass to the function in the logger

        \*\*kwargs: Dict[:class:`str`, Any]
            Keyword arguments to pass to the function in the logger

        Returns
        -------
        :class:`Any`
            The return value from running the corresponding function in the logger 
        """

        if (self.logger is not None):
            func = getattr(self.logger, funcName)
            return func(*args, **kwargs)


# Needed data model to inject into the .ini file
class RemapBlendModel():
    """
    Contains data for fixing a particular resource in a .ini file

    Parameters
    ----------
    iniFolderPath: :class:`str`
        The folder path to where the .ini file of the resource is located

    fixedBlendPaths: Dict[:class:`int`, Dict[:class:`str`, :class:`str`]]
        The file paths to the fixed RemapBlend.buf files for the resource :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for some resource
        * The inner keys are the names for the type of mod to fix to
        * The inner values are the file paths

    origBlendPaths: Optional[Dict[:class:`int`, :class:`str`]]
        The file paths to the Blend.buf files for the resource
        :raw-html:`<br />` :raw-html:`<br />`
        The keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for some resource :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    iniFolderPath: :class:`str`
        The folder path to where the .ini file of the resource is located

    fixedBlendPaths: Dict[:class:`int`, Dict[:class:`str`, :class:`str`]]
        The file paths to the fixed RemapBlend.buf files for the resource :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for some resource
        * The inner keys are the names for the type of mod to fix to
        * The inner values are the file paths

    origBlendPaths: Optional[Dict[:class:`int`, :class:`str`]]
        The file paths to the Blend.buf files for the resource :raw-html:`<br />` :raw-html:`<br />`

        The keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for the resource

    fullPaths: Dict[:class:`int`, Dict[:class:`str`, :class:`str`]]
        The absolute paths to the fixed RemapBlend.buf files for the resource :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for some resource
        * The inner keys are the names for the type of mod to fix to
        * The inner values are the file paths

    origFullPaths: Dict[:class:`int`, :class:`str`]
        The absolute paths to the Blend.buf files for the resource :raw-html:`<br />` :raw-html:`<br />`

        The keys are the indices that the Blend.buf file appears in the :class:`IfTemplate` for the resource
    """

    def __init__(self, iniFolderPath: str, fixedBlendPaths: Dict[int, Dict[str, str]], origBlendPaths: Optional[Dict[int, str]] = None):
        self.fixedBlendPaths = fixedBlendPaths
        self.origBlendPaths = origBlendPaths
        self.iniFolderPath = iniFolderPath

        self.fullPaths = {}
        self.origFullPaths = {}

        # retrieve the absolute paths
        for partIndex, partPaths in self.fixedBlendPaths.items():
            try:
                self.fullPaths[partIndex]
            except KeyError:
                self.fullPaths[partIndex] = {}

            for modName, path in partPaths.items():
                self.fullPaths[partIndex][modName] = FileService.absPathOfRelPath(path, iniFolderPath)

        if (self.origBlendPaths is not None):
            for partIndex in self.origBlendPaths:
                path = self.origBlendPaths[partIndex]
                self.origFullPaths[partIndex] = FileService.absPathOfRelPath(path, iniFolderPath)


class Version():
    """
    Class for handling game versions

    Parameters
    ----------
    versions: Optional[List[float]]
        The versions available

        **Default**: ``None``

    Attributes
    ----------
    _versionCache: :class:`LruCache`
        Cache to store the closest available versions based off the versions that the user searches :raw-html:`<br />` :raw-html:`<br />`

        * The keys in the `LRU cache`_ are the versions the user searches
        * The values in the  `LRU cache`_ are the corresponding versions available to the versions the user searches
    """

    def __init__(self, versions: Optional[List[float]] = None):
        if (versions is None):
            versions = []

        self._latestVersion: Optional[float] = None
        self._versionCache = LruCache()
        self.versions = versions

    @property
    def versions(self):
        """
        The available versions

        :getter: The versions in sorted ascending order
        :setter: Sets the new versions
        :type: List[float]
        """

        return self._versions
    
    @versions.setter
    def versions(self, newVersions: List[float]) -> List[float]:
        self.clear()

        self._versions = list(set(newVersions))
        self._versions.sort()
        if (self._versions):
            self._latestVersion = self._versions[-1]

    @property
    def latestVersion(self) -> Optional[float]:
        """
        The latest version available

        :getter: The latest version
        :type: Optional[float]
        """

        return self._latestVersion

    def clear(self):
        """
        Clears all the version data
        """

        self._versions = []
        self._latestVersion = None
        self._versionCache.clear()
    
    def _updateLatestVersion(self, newVersion: float):
        """
        Updates the latest version

        Parameters
        ----------
        newVersion: :class:`float`
            The new available version
        """

        if (self._latestVersion is None):
            self._latestVersion = newVersion
            return
        
        self._latestVersion = max(self._latestVersion, newVersion)

    def _add(self, newVersion: float):
        if (not self._versions or newVersion > self._versions[-1]):
            self._versions.append(newVersion)
        elif (newVersion < self._versions[0]):
            self._versions.insert(0, newVersion)
        else:
            Algo.binaryInsert(self._versions, newVersion, lambda v1, v2: v1 - v2, optionalInsert = True)

    def add(self, newVersion: float):
        """
        Adds a new version

        Parameters
        ----------
        newVersion: :class:`float`
            The new version to add
        """

        self._add(newVersion)
        self._updateLatestVersion(newVersion)

    def findClosest(self, version: Optional[float], fromCache: float = True) -> Optional[float]:
        """
        Finds the closest version available

        Parameters
        ----------
        version: Optional[:class:`float`]
            The version to be searched :raw-html:`<br />` :raw-html:`<br />`

            If This value is ``None``, then will assume we want the latest version :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        fromCache: :class:`float`
            Whether we want the result to be accessed from the cache :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        Optional[:class:`float`]
            The closest version available or ``None`` if there are no versions available
        """

        if (self._latestVersion is None):
            return None

        if (version is None):
            return self._latestVersion

        if (fromCache):
            try:
                return self._versionCache[version]
            except KeyError:
                pass

        found, ind = Algo.binarySearch(self._versions, version, lambda v1, v2: v1 - v2)

        result = 0
        if (found):
            result = self._versions[ind]
        elif (ind > 0):
            result = self._versions[ind - 1]
        else:
            result = self._versions[0]

        self._versionCache[version] = result
        return result


class ModAssets(Generic[T]):
    """
    Class to handle assets of any type for a mod

    .. note::
        This is a `bipartite graph` that maps assets to fix from to assets to fix to

    Parameters
    ----------
    repo: Dict[:class:`str`, Dict[:class:`str`, T]]
        The original source for any preset assets :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version number for the assets
        * The inner key is the name of the asset
        * The inner value is the content for the asset

    map: Optional[Dict[:class:`str`, Set[:class:`str`]]]
        The `adjacency list`_  that maps the assets to fix from to the assets to fix to using the predefined mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    repo: Dict[:class:`float`, Dict[:class:`str`, T]]
        The original source for any preset assets :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version number for the assets
        * The inner key is the name of the asset
        * The inner value is the content for the asset
    """

    def __init__(self, repo: Dict[float, Dict[str, T]], map: Optional[Dict[str, Set[str]]] = None):
        self._repo = repo
        self._fixFrom: Set[str] = set()
        self._fixTo: Set[str] = set()
        self._map = map

        if (self._map is None):
            self._map = {}  

        if (self._fixFrom is None):
            self._fixFrom = set()

        if (self._fixTo is None):
            self._fixTo = set()

        self._versions: Dict[str, Version] = {}

    @property
    def versions(self) -> Dict[str, Version]:
        """
        The game versions available for the assets :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the names of the assets
        * The values are versions for the asset

        :getter: Returns all the available game versions for the assets
        :type: Dict[:class:`str`, :class:`Version`]
        """

        return self._versions
    
    @property
    def fixFrom(self) -> Set[str]:
        """
        The names of the assets to fix from using the predefined mods

        :getter: Retrieves the names of assets used to fix from
        :type: Set[:class:`str`]
        """
        
        return self._fixFrom

    @property
    def fixTo(self) -> Set[str]:
        """
        The names of the assets to fix to using the predefined mods

        :getter: Retrives the names of assets to fix to
        :type: Set[:class:`str`]
        """

        return self._fixTo
    
    @property
    def map(self) -> Dict[str, Set[str]]:
        """
        The `adjacency list`_ used to map assets to fix from to assets to fix to

        :getter: Retrieves the `adjacency list`_
        :setter: Sets a new `adjacency list`_
        :type: Dict[:class:`str`, Set[:class:`str`]]
        """

        return self._map
    
    @map.setter
    def map(self, newMap: Dict[str, Set[str]]):
        self.clear(flush = True, clearMap = True)
        self.addMap(newMap)

    def clear(self, flush: bool = True, clearMap: bool = False):
        """
        Clears all the assets

        Parameters
        ----------
        flush: :class:`bool`
            Whether to flush out (reload) any cached data
            
            **Default**: ``False``

        clearMap: :class:`bool`
            Whether to clear out the mapping for the assets 

            **Default**: ``False``
        """

        if (flush):
            self._versions.clear()

        if (clearMap):
            self._fixFrom = set()
            self._fixTo = set()
            self._map = {}

    def loadFromPreset(self):
        """
        Reinitializes to load the predefined mods
        """

        map = self._map
        self.clear(clearMap = True)
        self.map = map

    @classmethod
    def updateMap(cls, srcMap: Dict[str, Set[str]], newMap: Dict[str, Set[str]]) -> Dict[str, Set[str]]:
        """
        Combines 2 maps together

        Parameters
        ----------
        srcMap: Dict[:class:`str`, Set[:class:`str`]]
            The map to be updates

        newMap: Dict[:class:`str`, Set[:class:`str`]]
            The new map to update ``srcMap``

        Returns
        -------
        Dict[:class:`str`, Set[:class:`str`]]
            The updated map
        """

        return DictTools.update(srcMap, newMap, combineDuplicate = lambda oldToAssets, newToAssets: oldToAssets.union(newToAssets))
    

    def _updateAssetContent(self, srcAsset: T, newAsset: T) -> T:
        """
        Combines the content of 2 assets

        Parameters
        ----------
        srcAsset: T
            The content of the asset to be updates

        newAsset: T
            The new asset to update into ``srcAsset`` 

        Returns
        -------
        T
            The updated asset
        """

        pass

    def _updateDupAssets(self, srcAsset: Dict[str, Any], newAsset: Dict[str, Any]):
        return DictTools.update(srcAsset, newAsset, combineDuplicate = self._updateAssetContent)
    
    def updateRepo(self, srcRepo: Dict[float, Dict[str, Any]], newRepo: Dict[float, Dict[str, Any]]) -> Dict[float, Dict[str, Any]]:
        """
        Updates the values in ``srcRepo``

        Parameters
        ----------
        srcRepo: Dict[:class:`float`, Dict[:class:`str`, Any]]
            The first repo to be combined

        newRepo: Dict[:class:`float`, Dict[:class:`str`, Any]]
            The second repo to be combined

        Returns
        -------
        Dict[:class:`float`, Dict[:class:`str`, Any]]
            The combined repo
        """

        result = DictTools.update(srcRepo, newRepo, combineDuplicate = self._updateDupAssets)
        return result

    def _addVersion(self, name: str, version: float):
        """
        Adds a new version for a particular asset

        Parameters
        ----------
        name: :class:`str`
            The name of the asset

        version: :class:`float`
            The game version
        """
        try:
            self._versions[name]
        except KeyError:
            self._versions[name] = Version()

        self._versions[name].add(version)

    def findClosestVersion(self, name: str, version: Optional[float] = None, fromCache: bool = True) -> float:
        """
        Finds the closest available game version from :attr:`ModStrAssets._toAssets` for a particular asset

        Parameters
        ----------
        name: :class:`str`
            The name of the asset to search

        version: Optional[:class:`float`]
            The game version to be searched :raw-html:`<br />` :raw-html:`<br />`

            If This value is ``None``, then will assume we want the latest version :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        fromCache: :class:`bool`
            Whether to use the result from the cache

            **Default**: ``None``

        Raises
        ------
        :class:`KeyError`
            The name for the particular asset is not found

        Returns
        -------
        :class:`float`
            The latest game version from the assets that corresponds to the desired version 
        """

        try:
            self._versions[name]
        except KeyError as e:
            raise KeyError(f"Asset name, '{name}', not found in the available versions") from e

        result = self._versions[name].findClosest(version, fromCache = fromCache)
        if (result is None):
            raise KeyError(f"No available versions for the asset by the name, '{name}'")

        return result
        
    def _partition(self, map: Dict[str, Set[str]], assets: Dict[float, Dict[str, Any]]) -> Tuple[Dict[str, Set[str]], Set[str], Set[str]]:
        """
        * Creates the `bipartition`_ for the assets to fix from vs the assets to fix to
        * Filters the mapping such that all the asset names in the new mapping exist in `assets`

        Parameters
        ----------
        map: Dict[:class:`str`, Set[:class:`str`]]
            The desired mapping for the assets for fixing

        assets: Dict[:class:`float`, Dict[:class:`str`, Any]]
            The source for all the assets :raw-html:`<br />` :raw-html:`<br />`

            * The outer key is the game version number for the assets
            * The first inner key is the name of the asset
            * The second inner key is the type of asset
            * The most inner value is the id for the asset (must be unique)

        Returns
        -------
        Tuple[Dict[:class:`str`, Set[:class:`str`]], Set[:class:`str`], Set[:class:`str`]]
            The following output is in the same order as listed below: :raw-html:`<br />` :raw-html:`<br />`

            #. The new mapping with all asset names being in `assets`
            #. The names of the assets to fix from
            #. The names of the assets to fix to
        """

        newMap = {}
        fixFrom = set()
        fixTo = set()

        vertices = set()
        visited = {}

        # retrieve all the vertices in the map
        for fromAsset in map:
            vertices.add(fromAsset)
            currentToAssets = map[fromAsset]
            vertices = vertices.union(currentToAssets)

        visited = {}
        for vertex in vertices:
            visited[vertex] = False

        # get all the vertices in the map that are visited in the assets repo
        for version in assets:
            versionAssets = assets[version]

            for assetName in versionAssets:
                if (assetName in vertices and not visited[assetName]):
                    visited[assetName] = True

        # creates the new sub-map and bipartition with vertices definitely being in the assets repo
        for fromAsset in map:
            if (not visited[fromAsset]):
                continue
            
            currentToAssets = map[fromAsset]
            newCurrentToAssets = set(filter(lambda toAsset: visited[toAsset], currentToAssets))

            if (not newCurrentToAssets):
                continue

            newMap[fromAsset] = newCurrentToAssets
            fixFrom.add(fromAsset)
            fixTo = fixTo.union(newCurrentToAssets)

        return (newMap, fixFrom, fixTo)
    
    def _updateVersions(self, assets: Dict[float, Dict[str, T]]):
        """
        Updates the versioning of the assets

        Parameters
        ----------
        assets: T
            The assets for checking the versioning
        """
        pass
    
    def addMap(self, assetMap: Dict[str, Set[str]], assets: Optional[Dict[float, Dict[str, T]]] = None):
        """
        Adds a new map to the existing map on how to retrieve the assets

        Parameters
        ----------
        assetMap: Dict[:class:`str`, Set[:class:`str`]]
            The new `adjacency list`_ used to map assets to fix from to assets to fix to

        assets: Optional[T]
            Any new assets that needs to be added/updated to the existing assets to support the given map

            **Default**: ``None``
        """

        if (assets is None):
            assets = {}

        self._repo = self.updateRepo(self._repo, assets)
        newAddMap, addFixFrom, addFixTo = self._partition(assetMap, self._repo)

        self._repo = self._repo
        if (not addFixFrom or not addFixTo):
            return

        self._map = self.updateMap(self._map, newAddMap)
        self._fixFrom = self._fixFrom.union(addFixFrom)
        self._fixTo = self._fixTo.union(addFixTo)

        # update the versions
        self._updateVersions(assets)


    def addMapping(self, fromAsset: str, toAssets: Set[str], assets: Any):
        """
        Adds a new mapping of how to fix the assets

        Parameters
        ----------
        fromAsset: :class:`str`
            The name of the asset to fix from

        toAssets: Set[:class:`str`]
            The names of the assets to fix to

        assets: Any
            Any new assets that needs to be added/updated to the existing assets to support the new mapping
        """

        map = {fromAsset: toAssets}
        self.addMap(map, assets)
    

class VGRemaps(ModAssets[Dict[str, VGRemap]]):
    """
    This class inherits from :class:`ModAssets`

    Class to handle Vertex Group Remaps for a mod

    Parameters
    ----------
    map: Optional[Dict[:class:`str`, Set[:class:`str`]]]
        The `adjacency list`_  that maps the assets to fix from to the assets to fix to using the predefined mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, map: Optional[Dict[str, Set[str]]] = None):
        super().__init__(VGRemapData, map = map)

        self._versions: Dict[str, Dict[str, Version]] = {}
        self.loadFromPreset()

    @property
    def versions(self) -> Dict[str, Version]:
        """
        The game versions available for the assets :raw-html:`<br />` :raw-html:`<br />`

        * The outer keys are the names of the assets to map from
        * The inner keys are the names of the assets to map to
        * The inner values are versions for the assets

        :getter: Returns all the available game versions for the assets
        :type: Dict[:class:`str`, Dict[:class:`str`, :class:`Version`]]
        """

        return self._versions

    def _updateAssetContent(self, asset1: Dict[str, VGRemap], asset2: Dict[str, VGRemap]) -> T:
        return DictTools.update(asset1, asset2)

    def loadFromPreset(self):
        super().loadFromPreset()
        self._updateVersions(self._repo)
    
    def _addVersion(self, fromAsset: str, toAsset: str, version: float):
        """
        Adds a new version for a particular asset

        Parameters
        ----------
        name: :class:`str`
            The name of the asset

        version: :class:`float`
            The game version
        """

        try:
            self._versions[fromAsset]
        except KeyError:
            self._versions[fromAsset] = {}

        try:
            self._versions[fromAsset][toAsset]
        except KeyError:
            self._versions[fromAsset][toAsset] = Version()

        self._versions[fromAsset][toAsset].add(version)

    def findClosestVersion(self, fromAsset: str, toAsset: str, version: Optional[float] = None, fromCache: bool = True) -> float:
        """
        Finds the closest available game version from :attr:`ModStrAssets._toAssets` for a particular asset

        Parameters
        ----------
        fromAsset: :class:`str`
            The name of the asset to map from

        toAsset: :class:`str`
            The name of the asset to map to

        version: Optional[:class:`float`]
            The game version to be searched :raw-html:`<br />` :raw-html:`<br />`

            If This value is ``None``, then will assume we want the latest version :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        fromCache: :class:`bool`
            Whether to use the result from the cache

            **Default**: ``None``

        Raises
        ------
        :class:`KeyError`
            The name for the particular asset is not found

        Returns
        -------
        :class:`float`
            The latest game version from the assets that corresponds to the desired version 
        """
        try:
            self._versions[fromAsset][toAsset]
        except KeyError as e:
            raise KeyError(f"Asset mapping from '{fromAsset}' to '{toAsset}' not found in the available versions") from e

        result = self._versions[fromAsset][toAsset].findClosest(version, fromCache = fromCache)
        if (result is None):
            KeyError("No available versions for the asset mapping")

        return result
    
    def get(self, fromAsset: str, toAsset: str, version: Optional[float] = None) -> str:
        """
        Retrieves the corresponding vertex group remap

        Parameters
        ----------
        fromAsset: :class:`str`
            The name of the asset to map from

        toAsset: :class:`str`
            The name of the asset to map to

        version: Optional[:class:`float`]
            The game version we want the remap to come from :raw-html:`<br />` :raw-html:`<br />`

            If This value is ``None``, then will retrieve the asset of the latest version. :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Raises
        ------
        :class:`KeyError`
            If the corresponding asset based on the search parameters is not found
            
        Returns
        -------
        :class:`str`
            The found asset
        """

        closestVersion = self.findClosestVersion(fromAsset, toAsset, version = version)
        result = self._repo[closestVersion][fromAsset][toAsset]
        return result

    def _updateVersions(self, assets: Dict[float, Dict[str, Dict[str, VGRemap]]]):
        assetNamesToUpdate = self.fixFrom.union(self.fixTo)

        for version, versionAssets in assets.items():
            for fromAssetName in versionAssets:
                if (fromAssetName not in assetNamesToUpdate):
                    continue

                fromAssets = versionAssets[fromAssetName]
                for toAssetName in fromAssets:
                    if (toAssetName not in assetNamesToUpdate):
                        continue

                    self._addVersion(fromAssetName, toAssetName, version)


class ModIdAssets(ModAssets[Dict[str, str]]):
    """
    This class inherits from :class:`ModAssets`

    Class to handle hashes, indices, and other string id type assets for a mod

    Parameters
    ----------
    repo: Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]]
        The original source for any preset assets :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version of the assets
        * The first inner key is the name of the asset
        * The second inner key is the type of asset
        * The most inner value is the id for the asset

        .. note::
            The id value for each asset should be unique

    map: Optional[Dict[:class:`str`, Set[:class:`str`]]]
        The `adjacency list`_  that maps the assets to fix from to the assets to fix to using the predefined mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, repo: Dict[float, Dict[str, Dict[str, str]]], map: Optional[Dict[str, Set[str]]] = None):
        super().__init__(repo, map = map)

        self._fromAssets: Dict[str, List[Tuple[str, str]]] = {}
        self._toAssets: Dict[float, Dict[str, Dict[str, str]]] = {}
        self.loadFromPreset()

    @property
    def fromAssets(self) -> Dict[str, Tuple[Set[str], str]]:
        """
        The assets to fix from :raw-html:`<br />` :raw-html:`<br />`

        * The keys are the ids for the asset
        * The values contains metadata about the assets to fix to where each tuple contains:

            # The names of the assets
            # The type of asset

        :getter: Returns the assets needed to be fixed
        :type: Dict[:class:`str`, Tuple[Set[:class:`str`], :class:`str`]]
        """

        return self._fromAssets
    
    @property
    def toAssets(self) -> Dict[float, Dict[str, Dict[str, str]]]:
        """
        The assets to fix to: :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the game version number for the assets
        * The first inner key is the name of the assets
        * The most inner key is the type of asset
        * The most inner value is the id for the asset

        :getter: Returns the new assets that will replace the old assets
        :type: Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]]
        """
        return self._toAssets

    def clear(self, flush: bool = True, clearMap: bool = False):
        self._fromAssets = {}
        self._toAssets = {}
        super().clear(flush = flush, clearMap = clearMap)

    def loadFromPreset(self):
        super().loadFromPreset()
        self._loadFromAssets()
        self._loadToAssets()

    def get(self, assetName: str, assetType: str, version: Optional[float] = None) -> str:
        """
        Retrieves the corresponding id asset from :attr:`ModStrAssets._toAssets`

        Parameters
        ----------
        assetName: :class:`str`
            The name of the assets we want

        assetType: :class:`str`
            The name of the type of asset we want.

        version: Optional[:class:`float`]
            The game version we want the asset to come from :raw-html:`<br />` :raw-html:`<br />`

            If This value is ``None``, then will retrieve the asset of the latest version. :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Raises
        ------
        :class:`KeyError`
            If the corresponding asset based on the search parameters is not found
            
        Returns
        -------
        :class:`str`
            The found asset
        """

        closestVersion = self.findClosestVersion(assetName, version = version)
        assets = self._toAssets[closestVersion]
        return assets[assetName][assetType]
    
    def replace(self, fromAsset: str, version: Optional[float] = None, toAssets: Optional[Union[str, Set[str]]] = None) -> Union[Optional[str], Dict[str, str]]:
        """
        Retrieves the corresponding asset to replace 'fromAsset'

        Parameters
        ----------
        fromAsset: :class:`str`
            The asset to be replaced

        version: Optional[:class:`float`]
            The game version we want the asset to come from :raw-html:`<br />` :raw-html:`<br />`

            If This value is ``None``, then will retrieve the asset of the latest version. :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        toAssets: Optional[Union[:class:`str`, Set[:class:`str`]]]
            The assets to used for replacement

        Returns
        -------
        Union[:class:`str`, Dict[:class:`str`, :class:`str`]]
            The corresponding assets for the fix to replace, if available :raw-html:`<br />` :raw-html:`<br />`

            The result is a string if the passed in parameter 'toAssets' is also a string :raw-html:`<br />` :raw-html:`<br />`
            
            Otherwise, the result is a dictionary such that: :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the names of the assets
            * The values are the corresponding asset ids to replace
        """

        if (fromAsset not in self._fromAssets):
            if (isinstance(toAssets, str)):
                return None
            else:
                return {}

        toAssetMetadata = self._fromAssets[fromAsset]
        toAssetType = toAssetMetadata[1]
        toAssetNames = toAssetMetadata[0]

        resultAsStr = False
        if (toAssets is not None and isinstance(toAssets, str)):
            toAssetNames = {toAssets}
            resultAsStr = True
        elif (toAssets is not None and toAssets):
            toAssetNames = toAssetNames.intersection(toAssets)

        result = {}
        for toAssetName in toAssetNames:
            try:
                currentReplacement = self.get(toAssetName, toAssetType, version = version)
            except KeyError:
                continue
            else:
                result[toAssetName] = currentReplacement

        if (resultAsStr):
            return result[toAssets]
        return result
    
    def _loadFromAssets(self):
        self._fromAssets = self._getFromAssets(self._map, self._repo)  

    def _loadToAssets(self):
        self._toAssets = self._getToAssets(self._fixTo, self._repo)
        
    def _updateAssetContent(self, srcAsset: Dict[str, str], newAsset: Dict[str, str]) -> Dict[str, str]:
        return DictTools.update(srcAsset, newAsset)

    def _getAssetChanges(self, oldAssets: Dict[float, Dict[str, Dict[str, str]]], newAssets: Dict[float, Dict[str, Dict[str, str]]]) -> Tuple[Dict[str, str], Dict[float, Dict[str, Dict[str, str]]], Dict[float, Dict[str, Dict[str, str]]]]:
        assetsToRemove = {}
        assetsToUpdate = {}
        changedIds = {}
        commonVersions = set(oldAssets.keys()).intersection(set(newAssets.keys()))
        
        for version in commonVersions:
            oldVersionAssets = oldAssets[version]
            newVersionAssets = newAssets[version]
            commonAssetNames = set(oldVersionAssets).intersection(set(newVersionAssets.keys()))

            for assetName in commonAssetNames:
                oldVersionNameAssets = oldVersionAssets[assetName]
                newVersionNameAssets = newVersionAssets[assetName]
                commonAssetTypes = set(oldVersionNameAssets.keys()).intersection(set(newVersionNameAssets.keys()))

                for assetType in commonAssetTypes:
                    oldAsset = oldVersionNameAssets[assetType]
                    newAsset = newVersionNameAssets[assetType]

                    if (oldAsset != newAsset):
                        assetsToRemove[version][assetName][assetType] = oldAsset
                        assetsToUpdate[version][assetName][assetType] = newAsset
                        changedIds[oldAsset] = newAsset

        return [changedIds, assetsToRemove, assetsToUpdate]

    @classmethod
    def _updateFromAssetsIds(self, fromAssets: Dict[str, Tuple[Set[str], str]], changedAssetIds: Dict[str, str]):
        for oldAssetId in changedAssetIds:
            newAssetId = changedAssetIds[oldAssetId]
            assetMetadata = fromAssets[oldAssetId]
            fromAssets.pop(oldAssetId)
            fromAssets[newAssetId] = assetMetadata

    @classmethod
    def _getFromAssets(cls, map: Dict[str, Set[str]], assets: Dict[float, Dict[str, Dict[str, str]]]) -> Dict[str, Tuple[Set[str], str]]:
        """
        Retrieves the assets to fix from

        Parameters
        ----------
        map: Dict[str, Set[str]]
            The mapping for fixing the assets

        assets: Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]]
            The source for all the assets :raw-html:`<br />` :raw-html:`<br />`

            * The outer key is the game version number for the assets
            * The first inner key is the name of the asset
            * The second inner key is the type of asset
            * The most inner value is the id for the asset (must be unique)

        Returns
        -------
        Dict[:class:`str`, Tuple[Set[:class:`str`], :class:`str`]]
            The assets to fix from :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the ids for the asset
            * The values contains metadata about the assets to fix to where each tuple contains:

                # The names of the assets
                # The type of asset

        """

        result = {}
        if (not map):
            return result

        invertedAssets = defaultdict(lambda: {})
        toAssets = defaultdict(lambda: set())

        for version in assets:
            versionAssets = assets[version]

            # get all the available assets to fix from
            for name in map:
                try:
                    asset = versionAssets[name]
                except KeyError:
                    continue
                else:
                    asset = DictTools.invert(asset)
                    DictTools.update(invertedAssets[name], asset)

            # get the available assets to fix to
            for name in map:
                toAssetNames = map[name]
                for toAssetName in toAssetNames:
                    try:
                        asset = versionAssets[toAssetName]
                    except:
                        continue
                    else:
                        toAssets[toAssetName] = toAssets[toAssetName].union(set(asset.keys()))

        # organize the assets
        for fromAssetName in invertedAssets:
            asset = invertedAssets[fromAssetName]
            toAssetNames = map[fromAssetName]

            for assetId in asset:
                assetType = asset[assetId]
                toNames = set()

                for toAssetName in toAssetNames:
                    toAssetTypes = toAssets[toAssetName]
                    if (assetType in toAssetTypes):
                        toNames.add(toAssetName)

                metadata = (toNames, assetType)
                result[assetId] = metadata

        return result
    
    @classmethod
    def _removeToAssets(cls, toAssets: Dict[float, Dict[str, Dict[str, str]]], assetsToRemove: Dict[float, Dict[str, Dict[str, str]]]):
        for version in toAssets:
            versionAssets = toAssets[version]
            
            for name in versionAssets:
                currentAssets = versionAssets[name]

                for type in currentAssets:
                    try:
                        assetsToRemove[version][name][type]
                    except:
                        continue
                    else:
                        toAssets[version][name].pop(type)

                if (not toAssets[version][name]):
                    toAssets[version].pop(name)

            if (not toAssets[version]):
                toAssets.pop(version)
    
    def _getToAssets(self, assetNames: Set[str], assets: Dict[float, Dict[str, Dict[str, str]]]) -> Dict[float, Dict[str, Dict[str, str]]]:
        """
        Retrieves the assets to fix to

        Parameters
        ----------
        assetNames: Set[:class:`str`]
            The names of the assets to fix to

        assets: Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]]
            The source for all the assets :raw-html:`<br />` :raw-html:`<br />`

            * The outer key is the game version number for the assets
            * The first inner key is the name of the asset
            * The second inner key is the type of asset
            * The most inner value is the id for the asset (must be unique)

        Returns
        -------
        Dict[:class:`float`, Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]]
            The assets to fix to  :raw-html:`<br />` :raw-html:`<br />`

            * The outer key is the game version number for the assets
            * The first inner key is the name of the asset
            * The second inner key is the type of asset
            * The most inner value is the id for the asset (must be unique)
        """

        result = {}
        if (not assetNames):
            return result
        
        prevToAssets = defaultdict(lambda: {})

        for version, versionAssets in assets.items():
            currentToAssets = {}

            for name in assetNames:
                try:
                    asset = versionAssets[name]
                except KeyError:
                    continue
                else:
                    prevAsset = prevToAssets[name]
                    DictTools.update(prevAsset, asset)
                    
                    if (prevAsset):
                        currentToAssets[name] = prevAsset
                        self._addVersion(name, version)

            if (currentToAssets):
                result[version] = currentToAssets

        return result


    def addMap(self, assetMap: Dict[str, Set[str]], assets: Optional[Dict[float, Dict[str, Dict[str, str]]]] = None):
        super().addMap(assetMap, assets = assets)
        if (assets is None):
            assets = {}

        changedIds, assetsIdsToRemove, assetsIdsToUpdate = self._getAssetChanges(self._repo, assets)
        self._repo = self.updateRepo(self._repo, assets)
        newAddMap, addFixFrom, addFixTo = self._partition(assetMap, self._repo)

        self._repo = self._repo
        if (not addFixFrom or not addFixTo):
            return

        self._map = self.updateMap(self._map, newAddMap)
        self._fixFrom = self._fixFrom.union(addFixFrom)
        self._fixTo = self._fixTo.union(addFixTo)

        # update the assets to fix from
        self._updateFromAssetsIds(self._fromAssets, changedIds)
        addFromAssets = self._getFromAssets(newAddMap, self._repo)
        DictTools.update(self._fromAssets, addFromAssets)

        # update the assets to fix to
        self._removeToAssets(self._toAssets, assetsIdsToRemove)

        addToAssetNames = set(map(lambda versionAssets: versionAssets.keys(), assetsIdsToUpdate.values()))
        addToAssetNames = addToAssetNames.union(addFixTo)
        addToAssets = self._getToAssets(addToAssetNames, self._repo)

        DictTools.update(self._toAssets, addToAssets, combineDuplicate = self._updateDupAssets)
        

class Hashes(ModIdAssets):
    """
    This class inherits from :class:`ModDictStrAssets`
    
    Class for managing hashes for a mod

    Parameters
    ----------
    map: Optional[Dict[:class:`str`, Set[:class:`str`]]]
        The `adjacency list`_  that maps the hashes to fix from to the hashes to fix to using the predefined mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, map: Optional[Dict[str, Set[str]]] = None):
        super().__init__(HashData, map = map)


class Indices(ModIdAssets):
    """
    This class inherits from :class:`ModDictStrAssets`
    
    Class for managing indices for a mod

    Parameters
    ----------
    map: Optional[Dict[:class:`str`, Set[:class:`str`]]]
        The `adjacency list`_  that maps the indices to fix from to the indices to fix to using the predefined mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
    """

    def __init__(self, map: Optional[Dict[str, Set[str]]] = None):
        super().__init__(IndexData, map = map)


class ModType():
    """
    Class for defining a generic type of mod

    Parameters
    ----------
    name: :class:`str`
        The default name for the type of mod

    check: Union[:class:`str`, `Pattern`_, Callable[[:class:`str`], :class:`bool`]]
        The specific check used to identify the .ini file belongs to the specific type of mod when checking arbitrary line in a .ini file :raw-html:`<br />` :raw-html:`<br />`

        #. If this argument is a string, then will check if a line in the .ini file equals to this argument
        #. If this argument is a regex pattern, then will check if a line in the .ini file matches this regex pattern
        #. If this argument is a function, then will check if a line in the .ini file will make the function for this argument return `True`

    hashes: Optional[:class:`Hashes`]
        The hashes related to the mod and its fix :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will create a new, empty :class:`Hashes` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``
        

    indices Optional[:class:`Indices`]
        The indices related to the mod and its fix :raw-html:`<br />` :raw-html:`<br />`

        If this ``None``, then will create a new , emtpy :class:`Indices` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    aliases: Optional[List[:class:`str`]]
        Other alternative names for the type of mod :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    vgRemaps: Optional[:class:`VGRemaps`]
        Maps the blend indices from the vertex group of one mod to another mod :raw-html:`<br />`

        If this value is ``None``, then will create a new, empty :class:`VGRemaps` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    name: :class:`str`
        The default name for the type of mod

    check: Union[:class:`str`, `Pattern`_, Callable[[:class:`str`], :class:`bool`]]
        The specific check used to identify the .ini file belongs to the specific type of mod when checking arbitrary line in a .ini file

    hashes: :class:`Hashes`
        The hashes related to the mod and its fix

    indices :class:`Indices`
        The indices related to the mod and its fix

    vgRemaps: :class:`VGRemaps`
        The repository that stores the mapping for remapping vertex group blend indices of the mod to the vertex group blend indices of another mod

    aliases: Optional[List[:class:`str`]]
        Other alternative names for the type of mod
    """

    def __init__(self, name: str, check: Union[str, Pattern, Callable[[str], bool]], hashes: Optional[Hashes], indices: Optional[Indices] = None, aliases: Optional[List[str]] = None, vgRemaps: Optional[VGRemaps] = None):
        self.name = name
        if (hashes is None):
            hashes = Hashes()

        if (indices is None):
            indices = Indices()

        self.hashes = hashes
        self.indices = indices

        self.check = check
        if (isinstance(check, str)):
            self._check = lambda line: line == check
        elif (callable(check)):
            self._check = check
        else:
            self._check = lambda line: bool(check.search(line))
        
        if (aliases is None):
            aliases = []
        self.aliases = ListTools.getDistinct(aliases)
        
        self._maxVgIndex = None
        if (vgRemaps is None):
            vgRemaps = VGRemaps()
        self.vgRemaps = vgRemaps

    def isName(self, name: str) -> bool:
        """
        Determines whether a certain name matches with the names defined for this type of mod

        Parameters
        ----------
        name: :class:`str`
            The name being searched

        Returns
        -------
        :class:`bool`
            Whether the searched name matches with the names for this type of mod
        """

        name = name.lower()
        if (self.name.lower() == name):
            return True
        
        for alias in self.aliases:
            if (alias.lower() == name):
                return True

        return False
    
    def isType(self, iniLine: str) -> bool:
        """
        Determines whether a line in the .ini file correponds with this mod type

        Parameters
        ----------
        iniLine: :class:`str`
            An arbitrary line in a .ini file

        Returns
        -------
        :class:`bool`
            Whether the line in the .ini file corresponds with this type of mod
        """

        return self._check(iniLine)
    

    def getModsToFix(self) -> Set[str]:
        """
        Retrieves the names of the mods this mod type will fix to

        Returns
        -------
        Set[:class:`str`]
            The names of the mods to fix to
        """

        result = set()
        result = result.union(self.hashes.fixTo)
        result = result.union(self.indices.fixTo)
        result = result.union(self.vgRemaps.fixTo)
        return result
    
    def getVGRemap(self, modName: str, version: Optional[float] = None) -> VGRemap:
        """
        Retrieves the corresponding Vertex Group Remap

        .. warning::
            This function assumes that the specified map :attr:`ModType.vgRemaps` (:attr:`VGRemaps.map`) contains :attr:`ModType.name` (the name of this mod type) as a mod to map from

        Parameters
        ----------
        modName: :class:`str`
            The name of the mod to map to

        version: Optional[:class:`float`]
            The specific game version we want for the remap :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will get the latest version of the remap :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns 
        -------
        :class:`VGRemap`
            The corresponding remap
        """

        return self.vgRemaps.get(self.name, modName, version = version)

    def getHelpStr(self) -> str:
        modTypeHeading = Heading(self.name, 8, "-")

        currentHelpStr = f"{modTypeHeading.open()}"
        currentHelpStr += f"\n\nname: {self.name}"
        
        if (self.aliases):
            aliasStr = ", ".join(self.aliases)
            currentHelpStr += f"\naliases: {aliasStr}"

        if (isinstance(self.check, str)):
            currentHelpStr += f"\ndescription: check if the .ini file contains the section named, '{self.check}'"
        elif (not callable(self.check)):
            currentHelpStr += f"\ndescription: check if the .ini file contains a section matching the regex, {self.check.pattern}"

        currentHelpStr += f"\n\n{modTypeHeading.close()}"
        return currentHelpStr


class ModTypes(Enum):
    """
    The supported types of mods that can be fixed

    Attributes
    ----------
    Raiden: :class:`ModType`
        **Raiden mods** :raw-html:`<br />`

        Checks if the .ini file contains a section with the regex ``[TextureOverride.*(Raiden|Shogun).*Blend]``
    """

    Raiden = ModType("Raiden", re.compile(r"^\s*\[\s*TextureOverride.*(Raiden|Shogun)((?!RemapBlend).)*Blend.*\s*\]"),
                     hashes = Hashes(map = {"Raiden": {"RaidenBoss"}}), indices = Indices(),
                     aliases = ["Ei", "RaidenEi", "Shogun", "RaidenShogun", "RaidenShotgun", "Shotgun", "CrydenShogun", "Cryden", "SmolEi"], 
                     vgRemaps = VGRemaps(map = {"Raiden": {"RaidenBoss"}}))
    
    Jean = ModType("Jean", re.compile(r"^\s*\[\s*TextureOverride.*(Jean)((?!(RemapBlend|CN)).)*Blend.*\s*\]"),
                   Hashes(map = {"Jean": {"JeanCN"}}), Indices(map = {"Jean": {"JeanCN"}}),
                   aliases = ["ActingGrandMaster", "KleesBabySitter"],
                   vgRemaps = VGRemaps(map = {"Jean": {"JeanCN"}}))
    
    JeanCN = ModType("JeanCN", re.compile(r"^\s*\[\s*TextureOverride.*(JeanCN)((?!RemapBlend).)*Blend.*\s*\]"),
                   Hashes(map = {"JeanCN": {"Jean"}}), Indices(map = {"JeanCN": {"Jean"}}),
                   aliases = ["ActingGrandMaster", "KleesBabySitter"],
                   vgRemaps = VGRemaps(map = {"JeanCN": {"Jean"}}))
    
    Amber = ModType("Amber", re.compile(r"^\s*\[\s*TextureOverride.*(Amber)((?!(RemapBlend|CN)).)*Blend.*\s*\]"),
                    Hashes(map = {"Amber": {"AmberCN"}}),Indices(map = {"Amber": {"AmberCN"}}),
                    aliases = ["BaronBunny", "ColleisBestie"],
                    vgRemaps = VGRemaps(map = {"Amber": {"AmberCN"}}))

    AmberCN = ModType("AmberCN", re.compile(r"^\s*\[\s*TextureOverride.*(AmberCN)((?!RemapBlend).)*Blend.*\s*\]"),
                    Hashes(map = {"AmberCN": {"Amber"}}),Indices(map = {"AmberCN": {"Amber"}}),
                    aliases = ["BaronBunnyCN", "ColleisBestieCN"],
                    vgRemaps = VGRemaps(map = {"AmberCN": {"Amber"}}))
    
    Mona = ModType("Mona", re.compile(r"^\s*\[\s*TextureOverride.*(Mona)((?!(RemapBlend|CN)).)*Blend.*\s*\]"),
                   Hashes(map = {"Mona": {"MonaCN"}}),Indices(map = {"Mona": {"MonaCN"}}),
                   aliases = ["NoMora", "BigHat"],
                   vgRemaps = VGRemaps(map = {"Mona": {"MonaCN"}}))
    
    MonaCN = ModType("MonaCN", re.compile(r"^\s*\[\s*TextureOverride.*(MonaCN)((?!RemapBlend).)*Blend.*\s*\]"),
                   Hashes(map = {"MonaCN": {"Mona"}}),Indices(map = {"MonaCN": {"Mona"}}),
                   aliases = ["NoMora", "BigHat"],
                   vgRemaps = VGRemaps(map = {"MonaCN": {"Mona"}}))
    
    Rosaria = ModType("Rosaria", re.compile(r"^\s*\[\s*TextureOverride.*(Rosaria)((?!(RemapBlend|CN)).)*Blend.*\s*\]"),
                      Hashes(map = {"Rosaria": {"RosariaCN"}}), Indices(map = {"Rosaria": {"RosariaCN"}}),
                      aliases = ["GothGirl"],
                      vgRemaps = VGRemaps(map = {"Rosaria": {"RosariaCN"}}))
    
    RosariaCN = ModType("RosariaCN", re.compile(r"^\s*\[\s*TextureOverride.*(RosariaCN)((?!RemapBlend).)*Blend.*\s*\]"),
                      Hashes(map = {"RosariaCN": {"Rosaria"}}), Indices(map = {"RosariaCN": {"Rosaria"}}),
                      aliases = ["GothGirl"],
                      vgRemaps = VGRemaps(map = {"RosariaCN": {"Rosaria"}}))
    
    Keqing = ModType("Keqing", re.compile(r"^\s*\[\s*TextureOverride.*(Keqing)((?!(RemapBlend|Opulent)).)*Blend.*\s*\]"),
                   Hashes(map = {"Keqing": {"KeqingOpulent"}}),Indices(map = {"Keqing": {"KeqingOpulent"}}),
                   aliases = ["Kequeen"],
                   vgRemaps = VGRemaps(map = {"Keqing": {"KeqingOpulent"}}))

    KeqingOpulent = ModType("KeqingOpulent", re.compile(r"^\s*\[\s*TextureOverride.*(KeqingOpulent)((?!RemapBlend).)*Blend.*\s*\]"),
            Hashes(map = {"KeqingOpulent": {"Keqing"}}),Indices(map = {"KeqingOpulent": {"Keqing"}}),
            aliases = ["LanternRiteKeqing", "KeqingLaternRite", "CuterKequeen", "LanternRiteKequeen", "KequeenLanternRite", "KequeenOpulent"],
            vgRemaps = VGRemaps(map = {"KeqingOpulent": {"Keqing"}}))
    
    Ningguang = ModType("Ningguang", re.compile(r"^\s*\[\s*TextureOverride.*(Ningguang)((?!(RemapBlend|Orchid)).)*Blend.*\s*\]"),
                   Hashes(map = {"Ningguang": {"NingguangOrchid"}}),Indices(map = {"Ningguang": {"NingguangOrchid"}}),
                   aliases = ["GeoMommy"],
                   vgRemaps = VGRemaps(map = {"Ningguang": {"NingguangOrchid"}}))
    
    NingguangOrchid = ModType("NingguangOrchid", re.compile(r"^\s*\[\s*TextureOverride.*(NingguangOrchid)((?!RemapBlend).)*Blend.*\s*\]"),
                    Hashes(map = {"NingguangOrchid": {"Ningguang"}}),Indices(map = {"NingguangOrchid": {"Ningguang"}}),
                    aliases = ["GeoMommyOrchid"],
                    vgRemaps = VGRemaps(map = {"NingguangOrchid": {"Ningguang"}}))
    
    
    @classmethod
    def getAll(cls) -> Set[ModType]:
        """
        Retrieves a set of all the mod types available

        Returns
        -------
        Set[:class:`ModType`]
            All the available mod types
        """

        result = set()
        for modTypeEnum in cls:
            result.add(modTypeEnum.value)
        return result
    
    @classmethod
    def search(cls, name: str):
        """
        Searches a mod type based off the provided name

        Parameters
        ----------
        name: :class:`str`
            The name of the mod to search for

        Returns
        -------
        Optional[:class:`ModType`]
            The found mod type based off the provided name
        """

        result = None
        for modTypeEnum in cls:
            modType = modTypeEnum.value
            if (modType.isName(name)):
                result = modType
                break
        
        return result
    
    @classmethod
    def getHelpStr(cls) -> str:
        result = ""
        helpHeading = Heading("supported types of mods", 15)
        result += f"{helpHeading.open()}\n\nThe names/aliases for the mod types are not case sensitive\n\n"

        modTypeHelpTxt = []
        for modTypeEnum in cls:
            modType = modTypeEnum.value
            currentHelpStr = modType.getHelpStr()
            modTypeHelpTxt.append(currentHelpStr)

        modTypeHelpTxt = "\n".join(modTypeHelpTxt)
        result += f"{modTypeHelpTxt}\n\n{helpHeading.close()}"
        return result


# IfTemplate: Data class for the if..else template of the .ini file
class IfTemplate():
    """
    Data for storing information about a `section`_ in a .ini file

    :raw-html:`<br />`

    .. note::
        Assuming every `if/else` clause must be on its own line, we have that an :class:`IfTemplate` have a form looking similar to this:

        .. code-block:: ini
            :linenos:
            :emphasize-lines: 1,2,5,7,12,16,17

            ...(does stuff)...
            ...(does stuff)...
            if ...(bool)...
                if ...(bool)...
                    ...(does stuff)...
                else if ...(bool)...
                    ...(does stuff)...
                endif
            else ...(bool)...
                if ...(bool)...
                    if ...(bool)...
                        ...(does stuff)...
                    endif
                endif
            endif
            ...(does stuff)...
            ...(does stuff)...

        We split the above structure into parts where each part is either:

        #. **An If Part**: a single line containing the keywords "if", "else" or "endif" :raw-html:`<br />` **OR** :raw-html:`<br />`
        #. **A Content Part**: a group of lines that *"does stuff"*

        **Note that:** an :class:`ifTemplate` does not need to contain any parts containing the keywords "if", "else" or "endif". This case covers the scenario
        when the user does not use if..else statements for a particular `section`_
        
        Based on the above assumptions, we can assume that every ``[section]`` in a .ini file contains this :class:`IfTemplate`

    :raw-html:`<br />`

    .. container:: operations

        **Supported Operations:**

        .. describe:: for element in x

            Iterates over all the parts of the :class:`IfTemplate`, ``x``

        .. describe:: x[num]

            Retrieves the part from the :class:`IfTemplate`, ``x``, at index ``num``

        .. describe:: x[num] = newPart

            Sets the part at index ``num`` of the :class:`IfTemplate`, ``x``, to have the value of ``newPart``

    :raw-html:`<br />`

    Parameters
    ----------
    parts: List[Union[:class:`str`, Dict[:class:`str`, Any]]]
        The individual parts of how we divided an :class:`IfTemplate` described above

    calledSubCommands: Optional[Dict[:class:`int`, :class:`str`]]
        Any other sections that this :class:`IfTemplate` references
        :raw-html:`<br />` :raw-html:`<br />`
        The keys are the indices to the part in the :class:`IfTemplate` that the section is called :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    hashes: Optional[Set[:class:`str`]]
        The hashes this :class:`IfTemplate` references

        **Default**: ``None``

    indices: Optional[Set[:class:`str`]]
        The indices this :class:`IfTemplate` references

        **Default**: ``None``

    name: :class:`str`
        The name of the `section`_ for this :class:`IfTemplate`

        **Default**: ``""``

    Attributes
    ----------
    parts: List[Union[:class:`str`, Dict[:class:`str`, Any]]]
        The individual parts of how we divided an :class:`IfTemplate` described above

    calledSubCommands: Dict[:class:`int`, :class:`str`]
        Any other sections that this :class:`IfTemplate` references
        :raw-html:`<br />` :raw-html:`<br />`
        The keys are the indices to the part in the :class:`IfTemplate` that the section is called

    hashes: Set[:class:`str`]
        The hashes this :class:`IfTemplate` references

    indices: Set[:class:`str`]
        The indices this :class:`IfTemplate` references
    """

    def __init__(self, parts: List[Union[str, Dict[str, Any]]], calledSubCommands: Optional[Dict[int, str]] = None, hashes: Optional[Set[str]] = None, 
                 indices: Optional[Set[str]] = None, name: str = ""):
        self.name = name
        self.parts = parts
        self.calledSubCommands = calledSubCommands
        self.hashes = hashes
        self.indices = indices

        if (calledSubCommands is None):
            self.calledSubCommands = {}

        if (self.hashes is None):
            self.hashes = set()

        if (self.indices is None):
            self.indices = set()

    def __iter__(self):
        return self.parts.__iter__()
    
    def __getitem__(self, key: int) -> Union[str, Dict[str, Any]]:
        return self.parts[key]
    
    def __setitem__(self, key: int, value: Union[str, Dict[str, Any]]):
        self.parts[key] = value

    def add(self, part: Union[str, Dict[str, Any]]):
        """
        Adds a part to the :class:`ifTemplate`

        Parameters
        ----------
        part: Union[:class:`str`, Dict[:class:`str`, Any]]
            The part to add to the :class:`IfTemplate`
        """
        self.parts.append(part)

    # find(pred, postProcessor): Searches each part in the if template based on 'pred'
    def find(self, pred: Optional[Callable[[int, Union[str, Dict[str, Any]]], bool]] = None, postProcessor: Optional[Callable[[int, Union[str, Dict[str, Any]]], Any]] = None) -> Dict[int, Any]:
        """
        Searches the :class:`IfTemplate` for parts that meet a certain condition

        Parameters
        ----------
        pred: Optional[Callable[[:class:`IfTemplate`, :class:`int`, Union[:class:`str`, Dict[:class:`str`, Any]]], :class:`bool`]]
            The predicate used to filter the parts :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then this function will return all the parts :raw-html:`<br />` :raw-html:`<br />`

            The order of arguments passed into the predicate will be:

            #. The :class:`IfTemplate` that this method is calling from
            #. The index for the part in the :class:`IfTemplate`
            #. The current part of the :class:`IfTemplate` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        postProcessor: Optional[Callable[[:class:`IfTemplate`, :class:`int`, Union[:class:`str`, Dict[str, Any]]], Any]]
            A function that performs any post-processing on the found part that meets the required condition :raw-html:`<br />` :raw-html:`<br />`

            The order of arguments passed into the post-processor will be:

            #. The :class:`IfTemplate` that this method is calling from
            #. The index for the part in the :class:`IfTemplate`
            #. The current part of the :class:`IfTemplate` :raw-html:`<br />` :raw-html:`<br />`
        
            **Default**: ``None``

        Returns
        -------
        Dict[:class:`int`, Any]
            The filtered parts that meet the search condition :raw-html:`<br />` :raw-html:`<br />`

            The keys are the index locations of the parts and the values are the found parts
        """

        result = {}
        if (pred is None):
            pred = lambda ifTemplate, partInd, part: True

        if (postProcessor is None):
            postProcessor = lambda ifTemplate, partInd, part: part

        partsLen = len(self.parts)
        for i in range(partsLen):
            part = self.parts[i]
            if (pred(self, i, part)):
                result[i] = (postProcessor(self, i, part))

        return result
    
    def getMods(self, hashRepo: Hashes, indexRepo: Indices, version: Optional[float] = None) -> Set[str]:
        """
        Retrieves the corresponding mods the :class:`IfTemplate` will fix to

        Parameters
        ----------
        hashRepo: :class:`Hashes`
            The data source for the hashes

        indexRepo: :class:`Indices`
            The data source for the indices

        version: Optional[:class:`float`]
            What version we want to fix :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, will assume we want to fix to the latest version :raw-html:`<br />` :raw-html:`<br />`
            
             **Default**: ``None``

        Returns
        -------
        Set[:class:`str`]
            Names of all the types of mods the :class:`IfTemplate` will fix to
        """

        result = set()

        for hash in self.hashes:
            replacments = hashRepo.replace(hash, version = version)
            result = result.union(set(replacments.keys()))

        for index in self.indices:
            replacments = indexRepo.replace(index, version = version)
            result = result.union(set(replacments.keys()))

        return result


class IniSectionGraph():
    """
    Class for constructing a directed subgraph for how the `sections`_ in the .ini file are ran

    :raw-html:`<br />`

    .. note::
        * The nodes are the `sections`_ of the .ini file
        * The directed edges are the command calls from the `sections`_ , where the source of the edge is the caller
        and the sink of the edge is the callee

    Parameters
    ----------
    sections: Set[:class:`str`]
        Names of the desired `sections`_ we want our subgraph to have from the `sections`_ of the .ini file

    allSections: Dict[:class:`str`, :class:`IfTemplate`]
        All the `sections`_ for the .ini file

        :raw-html:`<br />`

        .. note::
            You can think of this as the `adjacency list`_ for the directed graph of all `sections`_ in the .ini file

    remapNameFunc: Optional[Callable[[:class:`str`, :class:`str`], :class:`str`]]
        Function to get the corresponding remap names for the section names :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will not get the remap names for the sections :raw-html:`<br />` :raw-html:`<br />`

        The parameters for the function are:

            #. Name of the `section`_
            #. Name fo the type of mod to fix
        
        **Default**: ``False``

    modsToFix: Optional[Set[:class:`str`]]
        The names of the mods that will be fixed by the .ini file :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    remapNameFunc: Optional[Callable[[:class:`str`], :class:`str`]]
        Function to get the corresponding remap names for the section names :raw-html:`<br />` :raw-html:`<br />`

        The parameters for the function are:

            #. Name of the `section`_
            #. Name fo the type of mod to fix
    """

    def __init__(self, targetSections: Union[Set[str], List[str]], allSections: Dict[str, IfTemplate], 
                 remapNameFunc: Optional[Callable[[str], str]] = None, modsToFix: Optional[Set[str]] = None):
        self._modsToFix = modsToFix
        if (modsToFix is None):
            self._modsToFix = {}
        
        self._setTargetSections(targetSections)
        self._sections: Dict[str, IfTemplate] = {}
        self._allSections = allSections
        self._remapNames: Dict[str, Dict[str, str]] = {}
        self._runSequence: List[Tuple[str, IfTemplate]] = []
        self.remapNameFunc = remapNameFunc

        self.build()

    @property
    def targetSections(self):
        """
        Names of the desired `sections`_ we want our subgraph to have from the `sections`_ of the .ini file

        :getter: The names of the desired `sections`_ we want in the subgraph
        :setter: Constructs a new subgraph based on the new desired `sections`_ we want
        :type: List[:class:`str`]
        """

        return self._targetSections
    
    def _setTargetSections(self, newTargetSections: Union[Set[str], List[str]]):
        self._targetSections = ListTools.getDistinct(newTargetSections, keepOrder = True)
    
    @targetSections.setter
    def targetSections(self, newTargetSections: Union[Set[str], List[str]]):
        self._setTargetSections(newTargetSections)
        self.build()

    @property
    def sections(self):
        """
        The `sections`_ that are part of the contructed subgraph based on the desired sections specified at :attr:`IniSectionGraph.targetSections`

        :raw-html:`<br />`

        .. note::
            You can think of this as the `adjacency list`_ for the subgraph

        :getter: All the `sections`_ for the subgraph
        :type: Dict[:class:`str`, :class:`IfTemplate`]
        """

        return self._sections
    
    @property
    def allSections(self):
        """
        All the `sections`_ of the .ini file

        :raw-html:`<br />`

        .. note::
            You can think of this as the `adjacency list`_ for the directed graph of all `sections`_ in the .ini file

        :getter: All the `sections`_ for the .ini file
        :setter: Constructs a new subgraph based on the new `sections`_ for the .ini file
        :type: Dict[:class:`str`, :class:`IfTemplate`]
        """

        return self._allSections
    
    @allSections.setter
    def allSections(self, newAllSections: Dict[str, IfTemplate]):
        self._allSections = newAllSections
        self.build()

    @property
    def remapNames(self):
        """
        The corresponding names for the `sections`_ that the fix will make :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the name of the original `section`_
        * The inner key is the name for the type of mod to fix
        * The inner value is the corresponding name for the `section`_ and mod type

        :getter: All the corresponding names for the `sections`_
        :type: Dict[:class:`str`, Dict[:class:`str`, :class:`str`]]
        """

        return self._remapNames
    
    @property
    def runSequence(self):
        """
        The order the `sections`_ will be ran

        :getter: Retrieves the order the `sections`_ will be ran
        :type: List[Tuple[:class:`str`, :class:`IfTemplate`]]
        """

        return self._runSequence
    
    @property
    def modsToFix(self):
        """
        The names of the mods that will be fixed by the .ini file

        :getter: Retrieves the names of the mods to fix
        :type: Set[:class:`str`]
        """

        return self._modsToFix

    def build(self, newTargetSections: Optional[Union[Set[str], List[str]]] = None, newAllSections: Optional[Dict[str, IfTemplate]] = None,
              newModsToFix: Optional[Set[str]] = None):
        """
        Performs the initialization for rebuilding the subgraph

        Parameters
        ----------
        newTargetSections: Optional[Set[:class:`str`], List[:class:`str`]]
            The new desired `sections`_ we want in our subgraph :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        newAllSections: Optional[Dict[:class:`str`, :class:`IfTemplate`]]
            The new `sections`_ for the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        newModsToFix: Optional[Set[:class:`str`]]
            The new desired names of the mods that we want to fix for the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        if (newTargetSections is not None):
            self._setTargetSections(newTargetSections)

        if (newAllSections is not None):
            self._allSections = newAllSections

        self.construct()
        if (self.remapNameFunc is not None):
            self.getRemapBlendNames(newModsToFix = newModsToFix)
        else:
            self._remapNames = {}


    def getSection(self, sectionName: str, raiseException: bool = True) -> Optional[IfTemplate]:
        """
        Retrieves the :class:`IfTemplate` for a certain `section`_

        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_

        raiseException: :class:`bool`
            Whether to raise an exception when the section's :class:`IfTemplate` is not found

        Raises
        ------
        :class:`KeyError`
            If the :class:`IfTemplate` for the `section`_ is not found and ``raiseException`` is set to ``True``

        Returns
        -------
        Optional[:class:`IfTemplate`]
            The corresponding :class:`IfTemplate` for the `section`_
        """
        try:
            ifTemplate = self._allSections[sectionName]
        except Exception as e:
            if (raiseException):
                raise KeyError(f"The section by the name '{sectionName}' does not exist") from e
            else:
                return None
        else:
            return ifTemplate

    def _dfsExplore(self, section: IfTemplate, visited: Dict[str, IfTemplate], runSequence: List[Tuple[str, IfTemplate]]):
        """
        The typical recursive implementation of `DFS`_ for exploring a particular `section`_ (node)

        Parameters
        ----------
        section: :class:`IfTemplate`
            The `section`_ that is currently being explored
        
        visited: Dict[:class:`str`, :class:`ifTemplate`]
            The `sections`_ that have already been visited

        runSequence: List[Tuple[:class:`str`, :class:`IfTemplate`]]
            The order the `sections`_ will be ran
        """

        calledSubCommands = section.calledSubCommands
        for partInd in calledSubCommands:
            subSection = calledSubCommands[partInd]
            if (subSection not in visited):
                neighbourSection = self.getSection(subSection)
                visited[subSection] = neighbourSection
                
                runSequence.append((subSection, neighbourSection))
                self._dfsExplore(neighbourSection, visited, runSequence)

    def construct(self) -> Dict[str, IfTemplate]:
        """
        Constructs the subgraph for the `sections`_ using `DFS`_

        Returns
        -------
        Dict[:class:`str`, :class:`IfTemplate`]
            The `sections` that are part of the subgraph
        """

        visited = {}
        runSequence = []
        sections = {}

        for sectionName in self._targetSections:
            ifTemplate = self.getSection(sectionName)
            sections[sectionName] = ifTemplate

        # perform the main DFS algorithm
        for sectionName in sections:
            section = sections[sectionName]

            if (sectionName not in visited):
                visited[sectionName] = section
                runSequence.append((sectionName, section))
                self._dfsExplore(section, visited, runSequence)

        self._sections = visited
        self._runSequence = runSequence
        return self._sections

    def getRemapBlendNames(self, newModsToFix: Optional[Set[str]] = None) -> Dict[str, Dict[str, str]]:
        """
        Retrieves the corresponding remap names of the sections made by this fix

        Parameters
        ----------
        newModsToFix: Optional[Set[:class:`str`]]
            The new desired names of the mods that we want to fix for the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Dict[:class:`str`, :class:`str`]
            The new names for the `sections`_ with the 'FixRemap' keyword added
        """

        result = {}
        if (newModsToFix is not None):
            self._modsToFix = newModsToFix

        if (not self._modsToFix):
            return result

        for sectionName in self._sections:
            for modName in self._modsToFix:
                try:
                    result[sectionName]
                except KeyError:
                    result[sectionName] = {}

                result[sectionName][modName] = self.remapNameFunc(sectionName, modName)

        self._remapNames = result
        return result
    
    def getCommonMods(self, hashRepo: Hashes, indexRepo: Indices, version: Optional[float] = None) -> Set[str]:
        """
        Retrieves the common mods to fix to based off all the :class:`IfTemplate`s in the graph

        Parameters
        ----------
        hashRepo: :class:`Hashes`
            The data source for all the hashes

        indexRepo: :class:`Indices`
            The data source for all the indices

        version: Optional[:class:`float`]
            The version we want to fix to :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will assume we want to fix to the latest version :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Set[:class:`str`]
            The common mods to fix to
        """

        result = set()

        for sectionName in self._sections:
            ifTemplate = self._sections[sectionName]
            ifTemplateMods = ifTemplate.getMods(hashRepo, indexRepo, version = version)

            if (not result):
                result = ifTemplateMods
            elif (ifTemplateMods):
                result = result.intersection(ifTemplateMods)

        return result



# IniFile: Class to handle .ini files
class IniFile(Model):
    """
    This class inherits from :class:`Model`

    Class for handling .ini files

    :raw-html:`<br />`

    .. note::
        We analyse the .ini file using Regex which is **NOT** the right way to do things
        since the modified .ini language that GIMI interprets is a **CFG** (context free grammer) and **NOT** a regular language.
   
        But since we are lazy and don't want make our own compiler with tokenizers, parsing algorithms (eg. SLR(1)), type checking, etc...
        this module should handle regular cases of .ini files generated using existing scripts (assuming the user does not do anything funny...)

    :raw-html:`<br />`

    Parameters
    ----------
    file: Optional[:class:`str`]
        The file path to the .ini file :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    logger: Optional[:class:`Logger`]
        The logger to print messages if necessary

    txt: :class:`str`
        Used as the text content of the .ini file if :attr:`IniFile.file` is set to ``None`` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ""

    modTypes: Optional[Set[:class:`ModType`]]
        The types of mods that the .ini file should belong to :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    modsToFix: Optional[Set[:class:`str`]]
        The names of the mods we want to fix to :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    defaultModType: Optional[:class:`ModType`]
        The type of mod to use if the .ini file has an unidentified mod type :raw-html:`<br />` :raw-html:`<br />`
        If this value is ``None``, then will skip the .ini file with an unidentified mod type :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    version: Optional[:class:`float`]
        The game version we want the .ini file to be compatible with :raw-html:`<br />` :raw-html:`<br />`

        If This value is ``None``, then will retrieve the hashes/indices of the latest version. :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    version: Optional[:class:`float`]
        The game version we want the .ini file to be compatible with :raw-html:`<br />` :raw-html:`<br />`

        If This value is ``None``, then will retrieve the hashes/indices of the latest version.

    _parser: `ConfigParser`_
        Parser used to parse very basic cases in a .ini file

    modTypes: Set[:class:`ModType`]
        The types of mods that the .ini file should belong to

    modsToFix: Set[:class:`str`]
        The names of the mods that we want to fix to

    _toFix: Set[:class:`str`]
        The names of the mods that will be fix to from this .ini file

    defaultModType: Optional[:class:`ModType`]
        The type of mod to use if the .ini file has an unidentified mod type

    _textureOverrideBlendRoot: Optional[:class:`str`]
        The name for the `section`_ containing the keywords: ``[.*TextureOverride.*Blend.*]``

    _sectionIfTemplates: Dict[:class:`str`, :class:`IfTemplate`]
        All the `sections`_ in the .ini file that can be parsed into an :class:`IfTemplate`

        For more info see :class:`IfTemplate`

        .. warning::
            The modified .ini language that GIMI uses introduces keywords that can be used before the key of a key-value pair :raw-html:`<br />`

            *eg. defining constants*

            .. code-block:: ini
                :linenos:

                [Constants]
                global persist $swapvar = 0
                global persist $swapscarf = 0
                global $active
                global $creditinfo = 0

            :raw-html:`<br />`

            `Sections`_ containing this type of pattern will not be parsed. But generally, these sections are irrelevant to fixing the Raiden Boss

    _resourceBlends: Dict[:class:`str`, :class:`IfTemplate`]
        `Sections`_ that are linked to 1 or more Blend.buf files.

        The keys are the name of the sections.

    _blendCommandsGraph: :class:`IniSectionGraph`
        All the `sections`_ that use some ``[Resource.*Blend.*]`` section.

    _nonBlendHashIndexCommandsGraph :class:`IniSectionGraph`
        All the `sections`_ that are not used by the ``[Resource.*Blend.*]`` sections and contains the target hashes/indices that need to be replaced

    _resourceCommandsGraph: :class:`IniSectionGraph`
        All the related `sections`_ to the ``[Resource.*Blend.*]`` sections that are used by `sections`_ related to the ``[TextureOverride.*Blend.*]`` sections.
        The keys are the name of the `sections`_.

    remapBlendModels: Dict[:class:`str`, :class:`RemapBlendModel`]
        The data for the ``[Resource.*RemapBlend.*]`` `sections`_ used in the fix

        The keys are the original names of the resource with the pattern ``[Resource.*Blend.*]``
    """

    ShortModTypeNameReplaceStr = "{{shortModTypeName}}"
    ModTypeNameReplaceStr = "{{modTypeName}}"
    Credit = f'\n; {ModTypeNameReplaceStr}remapped by NK#1321 and Albert Gold#2696. If you used it to remap your {ShortModTypeNameReplaceStr}mods pls give credit for "Nhok0169" and "Albert Gold#2696"\n; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support'

    _oldHeading = Heading(".*Boss Fix", 15, "-")
    _defaultHeading = Heading(".*Remap", 15, "-")

    Hash = "hash"
    Vb1 = "vb1"
    Handling = "handling"
    Draw = "draw"
    Resource = "Resource"
    Blend = "Blend"
    Run = "run"
    MatchFirstIndex = "match_first_index"
    RemapBlend = f"Remap{Blend}"
    RemapFix = f"RemapFix"

    # -- regex strings ---

    _textureOverrideBlendPatternStr = r"^\s*\[\s*TextureOverride.*" + Blend + r".*\s*\]"
    _fixedTextureOverrideBlendPatternStr = r"^\s*\[\s*TextureOverride.*" + RemapBlend + r".*\s*\]"
    
    # --------------------
    # -- regex objects ---
    _sectionPattern = re.compile(r"^\s*\[.*\]")
    _textureOverrideBlendPattern = re.compile(_textureOverrideBlendPatternStr)
    _fixedTextureOverrideBlendPattern = re.compile(_fixedTextureOverrideBlendPatternStr)
    _fixRemovalPattern = re.compile(f"(; {_oldHeading.open()}((.|\n)*?); {_oldHeading.close()[:-2]}(-)*)|(; {_defaultHeading.open()}((.|\n)*?); {_defaultHeading.close()[:-2]}(-)*)")
    _removalPattern = re.compile(f"^\s*\[.*(" + RemapBlend + "|" + RemapFix + r").*\]")
    _sectionRemovalPattern = re.compile(f".*(" + RemapBlend + "|" + RemapFix + r").*")

    # -------------------

    _ifStructurePattern = re.compile(r"\s*(endif|if|else)")

    def __init__(self, file: Optional[str] = None, logger: Optional[Logger] = None, txt: str = "", modTypes: Optional[Set[ModType]] = None, defaultModType: Optional[ModType] = None, 
                 version: Optional[float] = None, modsToFix: Optional[Set[str]] = None):
        super().__init__(logger = logger)

        self._filePath: Optional[FilePath] = None
        self.file = file
        self.version = version
        self._parser = configparser.ConfigParser(dict_type = ConfigParserDict, strict = False)

        self._fileLines = []
        self._fileTxt = ""
        self._fileLinesRead = False
        self._ifTemplatesRead = False
        self._setupFileLines(fileTxt = txt)

        if (modTypes is None):
            modTypes = set()
        if (modsToFix is None):
            modsToFix = set()

        self.defaultModType = defaultModType
        self.modTypes = modTypes
        self.modsToFix = modsToFix
        self._toFix: Set[str] = {}
        self._heading = self._defaultHeading.copy()

        self._isFixed = False
        self._setType(None)
        self._isModIni = False

        self._textureOverrideBlendRoot: Optional[str] = None
        self._textureOverrideBlendSectionName: Optional[str] = None
        self._sectionIfTemplates: Dict[str, IfTemplate] = {}
        self._resourceBlends: Dict[str, IfTemplate] = {}

        self._blendCommandsGraph = IniSectionGraph(set(), {}, remapNameFunc = self.getRemapBlendName)
        self._nonBlendHashIndexCommandsGraph = IniSectionGraph(set(), {}, remapNameFunc = self.getRemapFixName)
        self._resourceCommandsGraph = IniSectionGraph(set(), {}, remapNameFunc = self.getRemapResourceName)

        self.remapBlendModels: Dict[str, RemapBlendModel] = {}

    @property
    def filePath(self) -> Optional[FilePath]:
        """
        The path to the .ini file

        :getter: Returns the path to the file
        :type: Optional[:class:`FilePath`]
        """
        return self._filePath

    @property
    def file(self) -> Optional[str]:
        """
        The file path to the .ini file

        :getter: Returns the path to the file
        :setter: Sets the new path for the file
        :type: Optional[:class:`str`]
        """

        if (self._filePath is None):
            return None
        return self._filePath.path
    
    @file.setter
    def file(self, newFile: Optional[str]) -> str:
        if (newFile is not None and self._filePath is None):
            self._filePath = FilePath(newFile)
        elif (newFile is not None):
            self._filePath.path = newFile
        elif (self._filePath is not None):
            self._filePath = None

    @property
    def folder(self) -> str:
        """
        The folder where this .ini file resides :raw-html:`<br />` :raw-html:`<br />`

        If :attr:`IniFile.file` is set to ``None``, then will return the folder where this script is ran

        :getter: Retrieves the folder
        :type: :class:`str`
        """

        if (self._filePath is not None):
            return self._filePath.folder
        return CurrentDir

    @property
    def isFixed(self) -> bool:
        """
        Whether the .ini file has already been fixed

        :getter: Returns whether the .ini file has already been fixed
        :type: :class:`bool`
        """

        return self._isFixed
    
    @property
    def type(self) -> Optional[ModType]:
        """
        The type of mod the .ini file belongs to

        :getter: Returns the type of mod the .ini file belongs to
        :type: Optional[:class:`ModType`]
        """

        return self._type
    
    def _setType(self, newType: Optional[ModType]):
        self._type = newType
        self._heading.title = None
    
    @property
    def isModIni(self) -> bool:
        """
        Whether the .ini file belongs to a mod

        :getter: Returns whether the .ini file belongs to a mod
        :type: :class:`bool`
        """

        return self._isModIni
    
    @property
    def fileLinesRead(self) -> bool:
        """
        Whether the .ini file has been read

        :getter: Determines whether the .ini file has been read
        :type: :class:`bool`
        """

        return self._fileLinesRead
    
    @property
    def fileTxt(self) -> str:
        """
        The text content of the .ini file

        :getter: Returns the content of the .ini file
        :setter: Reads the new value for both the text content of the .ini file and the text lines of the .ini file 
        :type: :class:`str`
        """

        return self._fileTxt
    
    @fileTxt.setter
    def fileTxt(self, newFileTxt: str):
        self._fileTxt = newFileTxt
        self._fileLines = TextTools.getTextLines(self._fileTxt)

        self._fileLinesRead = True
        self._isFixed = False
        self._textureOverrideBlendRoot = None
        self._textureOverrideBlendSectionName = None

    @property
    def fileLines(self) -> List[str]:
        """
        The text lines of the .ini file :raw-html:`<br />` :raw-html:`<br />`

        .. note::
            For the setter, each line must end with a newline character (same behaviour as `readLines`_)

        :getter: Returns the text lines of the .ini file
        :setter: Reads the new value for both the text lines of the .ini file and the text content of the .ini file
        :type: List[:class:`str`]
        """

        return self._fileLines
    
    @fileLines.setter
    def fileLines(self, newFileLines: List[str]):
        self._fileLines = newFileLines
        self._fileTxt = "".join(self._fileLines)

        self._fileLinesRead = True
        self._isFixed = False
        self._textureOverrideBlendRoot = None
        self._textureOverrideBlendSectionName = None

    def clearRead(self, eraseSourceTxt: bool = False):
        """
        Clears the saved text read in from the .ini file

        .. note::
            If :attr:`IniFile.file` is set to ``None``, then the default run of this function
            with the argument ``eraseSourceTxt`` set to ``False`` will have no effect since the provided text from :attr:`IniFile._fileTxt` is the only source of data for the :class:`IniFile`

            If you also want to clear the above source text data, then run this function with the ``eraseSourceTxt`` argument set to ``True``

        Parameters
        ----------
        eraseSourceTxt: :class:`bool`
            Whether to erase the only data source for this class if :attr:`IniFile.file` is set to ``None``, see the note above for more info :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``
        """

        if (self._filePath is not None or eraseSourceTxt):
            self._fileLines = []
            self._fileTxt = ""
            self._fileLinesRead = False

            self._isFixed = False
            self._textureOverrideBlendRoot = None
            self._textureOverrideBlendSectionName = None

    def clear(self, eraseSourceTxt: bool = False):
        """
        Clears all the saved data for the .ini file

        .. note::
            Please see the note at :meth:`IniFile.clearRead`

        Parameters
        ----------
        eraseSourceTxt: :class:`bool`
            Whether to erase the only data source for this class if :attr:`IniFile.file` is set to ``None``, see the note at :meth:`IniFile.clearRead` for more info :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``
        """

        self.clearRead(eraseSourceTxt = eraseSourceTxt)
        self._heading = self._defaultHeading.copy()
        self._setType(None)
        self._isModIni = False

        self._ifTemplatesRead = False
        self._sectionIfTemplates = {}
        self._resourceBlends = {}

        self._blendCommandsGraph.build(newTargetSections = set(), newAllSections = {})
        self._nonBlendHashIndexCommandsGraph.build(newTargetSections = set(), newAllSections = {})
        self._resourceCommandsGraph.build(newTargetSections = set(), newAllSections = {})

        self.remapBlendModels = {}


    @property
    def availableType(self) -> Optional[ModType]:
        """
        Retrieves the type of mod identified for this .ini file

        .. note::
            This function is the same as :meth:`IniFile.type`, but will return :attr:`IniFile.defaultModType` if :meth:`IniFile.type` is ``None``

        Returns
        -------
        Optional[:class:`ModType`]
            The type of mod identified
        """

        if (self._type is not None):
            return self._type
        elif (self.defaultModType is not None):
            return self.defaultModType
        
        return None

    def read(self) -> str:
        """
        Reads the .ini file :raw-html:`<br />` :raw-html:`<br />`

        If :attr:`IniFile.file` is set to ``None``, then will read the existing value from :attr:`IniFile.fileTxt`

        Returns
        -------
        :class:`str`
            The text content of the .ini file
        """

        if (self._filePath is not None):
            self.fileTxt = FileService.read(self._filePath.path, "r", lambda filePtr: filePtr.read())
        return self._fileTxt
    
    def write(self) -> str:
        """
        Writes back into the .ini files based off the content in :attr:`IniFile._fileLines`

        Returns
        -------
        :class:`str`
            The text that is written to the .ini file
        """

        if (self._filePath is None):
            return self._fileTxt

        with open(self._filePath.path, "w", encoding = IniFileEncoding) as f:
            f.write(self._fileTxt)

        return self._fileTxt

    def _setupFileLines(self, fileTxt: str = ""):
        if (self._filePath is None):
            self.fileTxt = fileTxt
            self._fileLinesRead = True

    def readFileLines(self) -> List[str]:
        """
        Reads each line in the .ini file :raw-html:`<br />` :raw-html:`<br />`

        If :attr:`IniFile.file` is set to ``None``, then will read the existing value from :attr:`IniFile.fileLines`

        Returns
        -------
        List[:class:`str`]
            All the lines read from the .ini file
        """

        if (self._filePath is not None):
            self.fileLines = FileService.read(self._filePath.path, "r", lambda filePtr: filePtr.readlines())
        return self._fileLines

    def _readLines(func):
        """
        Decorator to read all the lines in the .ini file first before running a certain function

        All the file lines will be saved in :attr:`IniFile._fileLines`

        Examples
        --------
        .. code-block:: python
            :linenos:

            @_readLines
            def printLines(self):
                for line in self._fileLines:
                    print(f"LINE: {line}")
        """

        def readLinesWrapper(self, *args, **kwargs):
            if (not self._fileLinesRead):
                self.readFileLines()
            return func(self, *args, **kwargs)
        return readLinesWrapper
    
    def checkIsMod(self) -> bool:
        """
        Reads the entire .ini file and checks whether the .ini file belongs to a mod

        .. note::
            If the .ini file has already been parsed (eg. calling :meth:`IniFile.checkModType` or :meth:`IniFile.parse`), then

            you only need to read :meth:`IniFile.isModIni`

        Returns
        -------
        :class:`bool`
            Whether the .ini file is a .ini file that belongs to some mod
        """
        
        self.clearRead()
        section = lambda line: False
        self.getSectionOptions(section, postProcessor = lambda startInd, endInd, fileLines, sectionName, srcTxt: "")
        return self._isModIni
    
    def _checkModType(self, line: str):
        """
        Checks if a line of text contains the keywords to identify whether the .ini file belongs to the types of mods in :attr:`IniFile.modTypes` :raw-html:`<br />` :raw-html:`<br />`

        * If :attr:`IniFile.modTypes` is not empty, then will find the first :class:`ModType` that where the line makes :meth:`ModType.isType` return ``True``
        * Otherwise, will see if the line matches with the regex, ``[.*TextureOverride.*Blend.*]`` 

        Parameters
        ----------
        line: :class:`str`
            The text to check
        """

        if (not self._isModIni and self.defaultModType is not None and 
            self._textureOverrideBlendSectionName is None and self._textureOverrideBlendPattern.search(line)):
            self._isModIni = True
            self._textureOverrideBlendSectionName = self._getSectionName(line)

        if (self._textureOverrideBlendRoot is not None):
            return
        
        if (not self.modTypes and self._textureOverrideBlendPattern.search(line)):
            self._textureOverrideBlendRoot = self._getSectionName(line)
            self._isModIni = True
            return

        for modType in self.modTypes:
            if (modType.isType(line)):
                self._textureOverrideBlendRoot = self._getSectionName(line)
                self._setType(modType)
                self._isModIni = True
                break

    def _checkFixed(self, line: str):
        """
        Checks if a line of text matches the regex, ``[.*TextureOverride.*RemapBlend.*]`` ,to identify whether the .ini file has been fixed

        Parameters
        ----------
        line: :class:`str`
            The line of text to check
        """

        if (not self._isFixed and self._fixedTextureOverrideBlendPattern.search(line)):
            self._isFixed = True

    def _parseSection(self, sectionName: str, srcTxt: str, save: Optional[Dict[str, Any]] = None) -> Optional[Dict[str, str]]:
        """
        Regularly parses the key-value pairs of a certain `section`_

        The function parses uses `ConfigParser`_ to parse the `section`_.

        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_

        srcTxt: :class:`str`
            The text containing the entire `section`_

        save: Optional[Dict[:class:`str`, Any]]
            Place to save the parsed result for the `section`_  :raw-html:`<br />` :raw-html:`<br />`

            The result for the parsed `section`_ will be saved as a value in the dictionary while section's name will be used in the key for the dictionary :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Optional[Dict[:class:`str`, :class:`str`]]
            The result from parsing the `section`_

            .. note:: 
                If `ConfigParser`_ is unable to parse the section, then ``None`` is returned
        """

        result = None
        try:
            self._parser.read_string(srcTxt)
            result = dict(self._parser[sectionName])
        except Exception:
            return result

        try:
            save[sectionName] = result
        except TypeError:
            pass

        return result
    
    def _getSectionName(self, line: str) -> str:
        currentSectionName = line
        rightPos = currentSectionName.rfind("]")
        leftPos = currentSectionName.find("[")

        if (rightPos > -1 and leftPos > -1):
            currentSectionName = currentSectionName[leftPos + 1:rightPos]
        elif (rightPos > -1):
            currentSectionName = currentSectionName[:rightPos]
        elif (leftPos > -1):
            currentSectionName = currentSectionName[leftPos + 1:]

        return currentSectionName.strip()

    # retrieves the key-value pairs of a section in the .ini file. Manually parsed the file since ConfigParser
    #   errors out on conditional statements in .ini file for mods. Could later inherit from the parser (RawConfigParser) 
    #   to custom deal with conditionals
    @_readLines
    def getSectionOptions(self, section: Union[str, Pattern, Callable[[str], bool]], postProcessor: Optional[Callable[[int, int, List[str], str, str], Any]] = None, 
                          handleDuplicateFunc: Optional[Callable[[List[Any]], Any]] = None) -> Dict[str, Any]:
        """
        Reads the entire .ini file for a certain type of `section`_

        Parameters
        ----------
        section: Union[:class:`str`, `Pattern`_, Callable[[:class:`str`], :class:`bool`]]
            The type of section to find

            * If this argument is a :class:`str`, then will check if the line in the .ini file exactly matches the argument
            * If this argument is a `Pattern`_, then will check if the line in the .ini file matches the specified Regex pattern
            * If this argument is a function, then will check if the line in the .ini file passed as an argument for the function will make the function return ``True``

        postProcessor: Optional[Callable[[:class:`int`, :class:`int`, List[:class:`str`], :class:`str`, :class:`str`], Any]]
            Post processor used when a type of `section`_ has been found

            The order of arguments passed into the post processor will be:

            #. The starting line index of the `section`_ in the .ini file
            #. The ending line index of the `section`_ in the .ini file
            #. All the file lines read from the .ini file
            #. The name of the `section`_ found
            #. The entire text for the `section`_ :raw-html:`<br />` :raw-html:`<br />`

            **Default**: `None`

        handleDuplicateFunc: Optional[Callable[List[Any], Any]]
            Function to used to handle the case of multiple sections names :raw-html:`<br />` :raw-html:`<br />`

            If this value is set to ``None``, will keep all sections with the same names

            .. note::
                For this case, GIMI only keeps the first instance of all sections with same names

            :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Dict[:class:`str`, Any]
            The resultant `sections`_ found

            The keys are the names of the `sections`_ found and the values are the content for the `section`_,
        """

        sectionFilter = None
        if (isinstance(section, str)):
            sectionFilter = lambda line: line == section
        elif callable(section):
            sectionFilter = section
        else:
            sectionFilter = lambda line: section.search(line)

        if (postProcessor is None):
            postProcessor = lambda startInd, endInd, fileLines, sectionName, srcTxt: self._parseSection(sectionName, srcTxt)

        result = {}
        currentSectionName = None
        currentSectionToParse = None
        currentSectionStartInd = -1

        fileLinesLen = len(self._fileLines)

        for i in range(fileLinesLen):
            line = self._fileLines[i]
            self._checkFixed(line)
            self._checkModType(line)

            # process the resultant section
            if (currentSectionToParse is not None and self._sectionPattern.search(line)):
                currentResult = postProcessor(currentSectionStartInd, i, self._fileLines, currentSectionName, currentSectionToParse)
                if (currentResult is None):
                    continue

                # whether to keep sections with the same name
                try:
                    result[currentSectionName]
                except KeyError:
                    result[currentSectionName] = [currentResult]
                else:
                    result[currentSectionName].append(currentResult)

                currentSectionToParse = None
                currentSectionName = None
                currentSectionStartInd = -1

            elif (currentSectionToParse is not None):
                currentSectionToParse += f"{line}"

            # keep track of the found section
            if (sectionFilter(line)):
                currentSectionToParse = f"{line}"
                currentSectionName = self._getSectionName(currentSectionToParse)
                currentSectionStartInd = i

        # get any remainder section
        if (currentSectionToParse is not None):
            currentResult = postProcessor(currentSectionStartInd, fileLinesLen, self._fileLines, currentSectionName, currentSectionToParse)
            try:
                result[currentSectionName]
            except:
                result[currentSectionName] = [currentResult]
            else:
                result[currentSectionName].append(currentResult)

        if (handleDuplicateFunc is None):
            return result

        # handle the duplicate sections with the same names
        for sectionName in result:
            result[sectionName] = handleDuplicateFunc(result[sectionName])

        return result

    def _removeSection(self, startInd: int, endInd: int, fileLines: List[str], sectionName: str, srcTxt: str) -> Tuple[int, int]:
        """
        Retrieves the starting line index and ending line index of where to remove a certain `section`_ from the read lines of the .ini file

        Parameters
        ----------
        startInd: :class:`int`
            The starting line index of the `section`_

        endInd: :class:`int`
            The ending line index of the `section`_

        fileLines: List[:class:`str`]
            All the file lines read from the .ini file

        sectionName: :class:`str`
            The name of the `section`_

        srcTxt: :class:`str`
            The text content of the `section`_

        Returns
        -------
        Tuple[:class:`int`, :class:`int`]
            The starting line index and the ending line index of the `section`_ to remove
        """

        fileLinesLen = len(fileLines)
        if (endInd > fileLinesLen):
            endInd = fileLinesLen

        if (startInd > fileLinesLen):
            startInd = fileLinesLen

        return (startInd, endInd)
    
    def removeSectionOptions(self, section: Union[str, Pattern, Callable[[str], bool]]):
        """
        Removes a certain type of `section`_ from the .ini file

        Parameters
        ----------
        section: Union[:class:`str`, `Pattern`_, Callable[[:class:`str`], :class:`bool`]]
            The type of `section`_ to remove
        """

        rangesToRemove = self.getSectionOptions(section, postProcessor = self._removeSection)
        partIndices = []

        for sectionName, ranges in rangesToRemove.items():
            for range in ranges:
                partIndices.append(range)

        self.fileLines = TextTools.removeLines(self._fileLines, partIndices)

    def _hasIfTemplateAtts(self, ifTemplate: IfTemplate, partIndex: int, part: Union[str, Dict[str, Any]]) -> bool:
        return isinstance(part, dict) and (self._isIfTemplateSubCommand(part) or self._isIfTemplateHash(part) or self._isIfTemplateMatchFirstIndex(part))

    def _setupIfTemplateAtts(self, ifTemplate: IfTemplate, partIndex: int, part: Union[str, Dict[str, Any]]):
        """
        Setup the attributes for the :class:`IfTemplate`

        Parameters
        ----------
        ifTemplate: :class:`IfTemplate`
            The :class:`IfTemplate` we are working with

        partIndex: :class:`int`
            The index for the part of the :class:`IfTemplate` we are working with

        part: Union[:class:`str`, Dict[:class:`str`, Any]]
            The part of the :class:`IfTemplate` we are working with
        """

        if (self._isIfTemplateSubCommand(part)):
            ifTemplate.calledSubCommands[partIndex] = self._getIfTemplateSubCommand(part)
        
        if (self._isIfTemplateHash(part)):
            ifTemplate.hashes.add(self._getIfTemplateHash(part))

        if (self._isIfTemplateMatchFirstIndex(part)):
            ifTemplate.indices.add(self._getIfTemplateMatchFirstIndex(part))


    def _createIfTemplate(self, ifTemplateParts: List[Union[str, Dict[str, Any]]], name: str = "") -> IfTemplate:
        """
        Creates an :class:`IfTemplate`

        Parameters
        ----------
        ifTemplateParts: List[Union[:class:`str`, Dict[:class:`str`, Any]]]
            The parts in the :class:`IfTemplate`

        name: :class:`str`
            The name of the `section`_ for the :class:`IfTemplate`

        Returns
        -------
        :class:`IfTemplate`
            The created :class:`IfTemplate` based off the imaginary 
        """

        result = IfTemplate(ifTemplateParts, name = name)
        result.find(pred = self._hasIfTemplateAtts, postProcessor = self._setupIfTemplateAtts)

        return result

    def _processIfTemplate(self, startInd: int, endInd: int, fileLines: List[str], sectionName: str, srcTxt: str) -> IfTemplate:
        """
        Parses a `section`_ in the .ini file as an :class:`IfTemplate`

        .. note::
            See :class:`IfTemplate` to see how we define an 'IfTemplate'

        Parameters
        ----------
        startInd: :class:`int`
            The starting line index of the `section`_

        endInd: :class:`int`
            The ending line index of the `section`_

        fileLines: List[:class:`str`]
            All the file lines read from the .ini file

        sectionName: :class:`str`
            The name of the `section`_

        srcTxt: :class:`str`
            The text content of the `section`_

        Returns
        -------
        :class:`IfTemplate`
            The generated :class:`IfTemplate` from the `section`_
        """

        ifTemplate = []
        dummySectionName = "dummySection"
        currentDummySectionName = f"{dummySectionName}"
        replaceSection = ""
        atReplaceSection = False

        for i in range(startInd + 1, endInd):
            line = fileLines[i]
            isConditional = bool(self._ifStructurePattern.match(line))

            if (isConditional and atReplaceSection):
                currentDummySectionName = f"{dummySectionName}{i}"
                replaceSection = f"[{currentDummySectionName}]\n{replaceSection}"

                currentPart = self._parseSection(currentDummySectionName, replaceSection)
                if (currentPart is None):
                    currentPart = {}

                ifTemplate.append(currentPart)
                replaceSection = ""

            if (isConditional):
                ifTemplate.append(line)
                atReplaceSection = False
                continue
            
            replaceSection += line
            atReplaceSection = True

        # get any remainder replacements in the if..else template
        if (replaceSection != ""):
            currentDummySectionName = f"{dummySectionName}END{endInd}"
            replaceSection = f"[{currentDummySectionName}]\n{replaceSection}"
            currentPart = self._parseSection(currentDummySectionName, replaceSection)
            if (currentPart is None):
                currentPart = {}

            ifTemplate.append(currentPart)

        # create the if template
        result = self._createIfTemplate(ifTemplate, name = sectionName)
        return result
    

    def getIfTemplates(self, flush: bool = False) -> Dict[str, IfTemplate]:
        """
        Retrieves all the :class:`IfTemplate`s for the .ini file

        .. note::
            This is the same as :meth:`IniFile.readIfTemplates`, but uses caching

        Parameters
        ----------
        flush: :class:`bool`
            Whether to re-parse the :class:`IfTemplates`s instead of using the saved cached values

        Returns
        -------
        Dict[:class:`str`, :class:`IfTempalte`]
            The parsed :class:`IfTemplate`s :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the name of the :class:`IfTemplate`
            * The values are the corresponding :class:`IfTemplate`
        """

        if (not self._ifTemplatesRead or flush):
            self.readIfTemplates()
        return self._sectionIfTemplates

    def readIfTemplates(self) -> Dict[str, IfTemplate]:
        """
        Parses all the :class:`IfTemplate`s for the .ini file

        Returns
        -------
        Dict[:class:`str`, :class:`IfTempalte`]
            The parsed :class:`IfTemplate`s :raw-html:`<br />` :raw-html:`<br />`

            * The keys are the name of the :class:`IfTemplate`
            * The values are the corresponding :class:`IfTemplate`
        """

        self._sectionIfTemplates = self.getSectionOptions(self._sectionPattern, postProcessor = self._processIfTemplate, handleDuplicateFunc = lambda duplicates: duplicates[0])
        self._ifTemplatesRead = True
        return self._sectionIfTemplates 
    
    @classmethod
    def getMergedResourceIndex(cls, mergedResourceName: str) -> Optional[int]:
        """
        Retrieves the index number of a resource created by GIMI's ``genshin_merge_mods.py`` script

        Examples
        --------
        >>> IniFile.getMergedResourceIndex("ResourceCuteLittleEiBlend.8")
        8


        >>> IniFile.getMergedResourceIndex("ResourceCuteLittleEiBlend.Score.-100")
        -100


        >>> IniFile.getMergedResourceIndex("ResourceCuteLittleEiBlend.UnitTests")
        None


        >>> IniFile.getMergedResourceIndex("ResourceCuteLittleEiBlend")
        None

        Parameters
        ----------
        mergedResourceName: :class:`str`
            The name of the `section`_

        Returns
        -------
        Optional[:class:`int`]
            The index for the resource `section`_, if found and the index is an integer
        """
        result = None

        try:
            result = int(mergedResourceName.rsplit(".", 1)[-1])
        except:
            pass
            
        return result
    
    def _compareResources(self, resourceTuple1: Tuple[str, Optional[int]], resourceTuple2: Tuple[str, Optional[int]]) -> int:
        """
        Compare function used for sorting resources :raw-html:`<br />` :raw-html:`<br />`

        The order for sorting is the resources is:
        
        #. Resources that do are not suffixed by an index number
        #. Resource that are suffixed by an index number (see :meth:`IniFile.getMergedResourceIndex` for more info)

        Parameters
        ----------
        resourceTuple1: Tuple[:class:`str`, Optional[:class:`int`]]
            Data for the first resource in the compare function, contains:

            * Name of the resource
            * The index for the resource

        resourceTuple2: Tuple[:class:`str`, Optional[:class:`int`]]
            Data for the second resource in the compare function, contains:

            * Name of the resource
            * The index for the resource

        Returns
        -------
        :class:`int`
            The result for a typical compare function used in sorting

            * returns -1 if ``resourceTuple1`` should come before ``resourceTuple2``
            * returns 1 if ``resourceTuple1`` should come after ``resourceTuple2``
            * returns 0 if ``resourceTuple1`` is equal to ``resourceTuple2`` 
        """

        resourceKey1 = resourceTuple1[1]
        resourceKey2 = resourceTuple2[1]
        resource1MissingIndex = resourceKey1 is None
        resource2MissingIndex = resourceKey2 is None

        if (resource1MissingIndex):
            resourceKey1 = resourceTuple1[0]
        
        if (resource2MissingIndex):
            resourceKey2 = resourceTuple2[0]

        if ((resource1MissingIndex == resource2MissingIndex and resourceKey1 < resourceKey2) or (resource1MissingIndex and not resource2MissingIndex)):
            return -1
        elif ((resource1MissingIndex == resource2MissingIndex and resourceKey1 > resourceKey2) or (not resource1MissingIndex and resource2MissingIndex)):
            return 1
        
        return 0

    # Disabling the OLD ini
    def disIni(self, makeCopy: bool = False):
        """
        Disables the .ini file

        .. note::
            For more info, see :meth:`FileService.disableFile` 

        Parameters
        ----------
        makeCopy: :class:`bool`
            Whether to make a copy of the disabled .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``
        """

        if (self._filePath is None):
            return

        disabledPath = FileService.disableFile(self._filePath.path)
        if (makeCopy):
            FileService.copyFile(disabledPath, self._filePath.path)

    @classmethod
    def getFixedBlendFile(cls, blendFile: str, modName: str = "") -> str:
        """
        Retrieves the file path for the fixed RemapBlend.buf file

        Parameters
        ----------
        blendFile: :class:`str`
            The file path to the original Blend.buf file

        modName: :class:`str`
            The name of the mod to fix to

        Returns
        -------
        :class:`str`
            The file path of the fixed RemapBlend.buf file
        """

        blendFolder = os.path.dirname(blendFile)
        blendBaseName = os.path.basename(blendFile)
        blendBaseName = blendBaseName.rsplit(".", 1)[0]
        
        return os.path.join(blendFolder, f"{cls.getRemapBlendName(blendBaseName, modName = modName)}.buf")
    
    def getFixModTypeName(self) -> Optional[str]:
        """
        Retrieves the name of the type of mod corresponding to the .ini file to be used for the comment of the fix

        Returns
        -------
        Optional[:class:`str`]
            The name for the type of mod corresponding to the .ini file
        """
        if (self._type is None):
            return None
        return self._type.name.replace("\n", "").replace("\t", "")
    
    def getFixModTypeHeadingname(self):
        """
        Retrieves the name of the type of mod corresponding to the .ini file to be used in the header/footer divider comment of the fix

        Returns
        -------
        Optional[:class:`str`]
            The name for the type of mod to be displayed in the header/footer divider comment
        """

        modTypeName = self.getFixModTypeName()
        if (modTypeName is None):
            modTypeName = "GI"

        return modTypeName

    def getHeadingName(self):
        """
        Retrieves the title for the header of the divider comment of the fix

        Returns
        -------
        :class:`str`
            The title for the header of the divider comment
        """

        result = self.getFixModTypeHeadingname()
        if (result):
            result += " "

        return f"{result}Remap"

    def getFixHeader(self) -> str:
        """
        Retrieves the header text used to identify a code section has been changed by this fix
        in the .ini file

        Returns
        -------
        :class:`str`
            The header section comment to be used in the .ini file
        """
        
        if (self._heading.title is None):
            self._heading.title = self.getHeadingName()
        return f"; {self._heading.open()}"
    
    def getFixFooter(self) -> str:
        """
        Retrieves the footer text used to identify a code section has been changed by this fix
        in the .ini file

        Returns
        -------
        :class:`str`
            The footer section comment to be used in the .ini file
        """

        if (self._heading.title is None):
            self._heading.title = self.getHeadingName()
        return f"\n\n; {self._heading.close()}"
    
    def getFixCredit(self) -> str:
        """
        Retrieves the credit text for the code generated in the .ini file

        Returns
        -------
        :class:`str`
            The credits to be displayed in the .ini file
        """

        modTypeName = self.getFixModTypeName()
        shortModTypeName = modTypeName

        if (modTypeName is None):
            modTypeName = "Mod"
            shortModTypeName = ""

        if (modTypeName):
            modTypeName += " "
        
        if (shortModTypeName):
            shortModTypeName += " "
        
        return self.Credit.replace(self.ModTypeNameReplaceStr, modTypeName).replace(self.ShortModTypeNameReplaceStr, shortModTypeName)

    def _addFixBoilerPlate(func):
        """
        Decorator used to add the boilerplate code to identify a code section has been changed by this fix in the .ini file

        Examples
        --------
        .. code-block:: python
            :linenos:

            @_addFixBoilerPlate
            def helloWorld(self) -> str:
                return "Hello World"
        """

        def addFixBoilerPlateWrapper(self, *args, **kwargs):
            addFix = self.getFixHeader()
            addFix += self.getFixCredit()
            addFix += func(self, *args, **kwargs)
            addFix += self.getFixFooter()

            return addFix
        return addFixBoilerPlateWrapper
    
    @classmethod
    def getResourceName(cls, name: str) -> str:
        """
        Makes the name of a `section`_ to be used for the resource `sections`_ of a .ini file

        Examples
        --------
        >>> IniFile.getResourceName("CuteLittleEi")
        "ResourceCuteLittleEi"


        >>> IniFile.getResourceName("ResourceCuteLittleEi")
        "ResourceCuteLittleEi"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        Returns
        -------
        :class:`str`
            The name of the `section`_ as a resource in a .ini file
        """

        if (not name.startswith(cls.Resource)):
            name = f"{cls.Resource}{name}"
        return name
    
    @classmethod
    def removeResourceName(cls, name: str) -> str:
        """
        Removes the 'Resource' prefix from a section's name

        Examples
        --------
        >>> IniFile.removeResourceName("ResourceCuteLittleEi")
        "CuteLittleEi"


        >>> IniFile.removeResourceName("LittleMissGanyu")
        "LittleMissGanyu"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the 'Resource' prefix removed
        """

        if (name.startswith(cls.Resource)):
            name = name[len(cls.Resource):]

        return name
    
    @classmethod
    def getRemapBlendName(cls, name: str, modName: str = "") -> str:
        """
        Changes a `section`_ name to have the keyword 'RemapBlend' to identify that the `section`_
        is created by this fix

        Examples
        --------
        >>> IniFile.getRemapBlendName("EiTriesToUseBlenderAndFails", "Raiden")
        "EiTriesToUseRaidenRemapBlenderAndFails"


        >>> IniFile.getRemapBlendName("EiBlendsTheBlender", "Yae")
        "EiBlendsTheYaeRemapBlender"
    

        >>> IniFile.getRemapBlendName("ResourceCuteLittleEi", "Raiden")
        "ResourceCuteLittleEiRaidenRemapBlend"


        >>> IniFile.getRemapBlendName("ResourceCuteLittleEiRemapBlend", "Raiden")
        "ResourceCuteLittleEiRemapRaidenRemapBlend"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the added 'RemapBlend' keyword
        """

        nameParts = name.rsplit(cls.Blend, 1)
        namePartsLen = len(nameParts)

        remapName = f"{modName}{cls.RemapBlend}"
        if (namePartsLen > 1):
            name = remapName.join(nameParts)
        else:
            name += remapName

        return name
    
    @classmethod
    def getRemapFixName(cls, name: str, modName: str = "") -> str:
        """
        Changes a `section`_ name to have the suffix `RemapFix` to identify that the `section`_
        is created by this fix

        Examples
        --------
        >>> IniFile.getRemapFixName("EiIsDoneWithRemapFix", "Raiden")
        "EiIsDoneWithRaidenRemapFix"

        >>> IniFile.getRemapFixName("EiIsHappy", "Raiden")
        "EiIsHappyRaidenRemapFix"

        Parameters
        ----------
        name: :class:`str`
            The name of the `section`_

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the `section`_ with the added 'RemapFix' keyword
        """

        remapName = f"{modName}{cls.RemapFix}"
        if (name.endswith(remapName)):
            return name
        elif(name.endswith(cls.RemapFix)):
            return name[:len(cls.RemapFix)] + remapName

        return name + remapName

    @classmethod
    def getRemapResourceName(cls, name: str, modName: str = "") -> str:
        """
        Changes the name of a section to be a new resource that this fix will create

        .. note::
            See :meth:`IniFile.getResourceName` and :meth:`IniFile.getRemapBlendName` for more info

        Parameters
        ----------
        name: :class:`str`
            The name of the section

        modName: :class:`str`
            The name of the mod to fix :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``""``

        Returns
        -------
        :class:`str`
            The name of the section with the prefix 'Resource' and the keyword 'Remap' added
        """

        name = cls.getRemapBlendName(name, modName = modName)
        name = cls.getResourceName(name)
        return name

    def _isIfTemplateResource(self, ifTemplatePart: Dict[str, Any]) -> bool:
        """
        Whether the content for some part of a `section`_ contains the key 'vb1'

        Parameters
        ----------
        ifTemplatePart: Dict[:class:`str`, Any]
            The key-value pairs for some part in a `section`_

        Returns
        -------
        :class:`bool`
            Whether 'vb1' is contained in the part
        """

        return self.Vb1 in ifTemplatePart
    
    def _isIfTemplateDraw(self, ifTemplatePart: Dict[str, Any]) -> bool:
        """
        Whether the content for some part of a `section`_ contains the key 'draw'

        Parameters
        ----------
        ifTemplatePart: Dict[:class:`str`, Any]
            The key-value pairs for some part in a `section`_

        Returns
        -------
        :class:`bool`
            Whether 'draw' is contained in the part
        """


        return self.Draw in ifTemplatePart
    
    def _isIfTemplateHash(self, ifTemplatePart: Dict[str, Any]) -> bool:
        """
        Whether the content for some part of a `section`_ contains the key 'hash'

        Parameters
        ----------
        ifTemplatePart: Dict[:class:`str`, Any]
            The key-value pairs for some part in a section

        Returns
        -------
        :class:`bool`
            Whether 'hash' is contained in the part
        """
                
        return self.Hash in ifTemplatePart
    
    def _isIfTemplateMatchFirstIndex(self, ifTemplatePart: Dict[str, Any]) -> bool:
        """
        Whether the content for some part of a `section`_ contains the key 'match_first_index'

        Parameters
        ----------
        ifTemplatePart: Dict[:class:`str`, Any]
            The key-value pairs for some part in a section

        Returns
        -------
        :class:`bool`
            Whether 'match_first_index' is contained in the part
        """
                
        return self.MatchFirstIndex in ifTemplatePart
    
    def _isIfTemplateSubCommand(self, ifTemplatePart: Dict[str, Any]) -> bool:
        """
        Whether the content for some part of a `section`_ contains the key 'run'

        Parameters
        ----------
        ifTemplatePart: Dict[:class:`str`, Any]
            The key-value pairs for some part in a section

        Returns
        -------
        :class:`bool`
            Whether 'run' is contained in the part
        """
                
        return self.Run in ifTemplatePart
    
    def _getIfTemplateResourceName(self, ifTemplatePart: Dict[str, Any]) -> Any:
        """
        Retrieves the value from the key, 'vb1', for some part of a `section`_

        Parameters
        ----------
        ifTemplatePart: Dict[:class:`str`, Any]
            The key-value pairs for some part in a `section`_

        Returns
        -------
        Any
            The corresponding value for the key 'vb1'
        """

        return ifTemplatePart[self.Vb1]
    
    def _getIfTemplateSubCommand(self, ifTemplatePart: Dict[str, Any]) -> Any:
        """
        Retrieves the value from the key, 'run', for some part of a `section`_

        Parameters
        ----------
        ifTemplatePart: Dict[:class:`str`, Any]
            The key-value pairs for some part in a `section`_

        Returns
        -------
        Any
            The corresponding value for the key 'run'
        """

        return ifTemplatePart[self.Run]
    
    def _getIfTemplateHash(self, ifTemplatePart: Dict[str, Any]) -> Any:
        """
        Retrieves the value from the key, 'hash', for some part of a `section`_

        Parameters
        ----------
        ifTemplatePart: Dict[:class:`str`, Any]
            The key-value pairs for some part in a `section`_

        Returns
        -------
        Any
            The corresponding value for the key 'hash'
        """

        return ifTemplatePart[self.Hash]
    
    def _getIfTemplateMatchFirstIndex(self, ifTemplatePart: Dict[str, Any]) -> Any:
        """
        Retrieves the value from the key, 'match_first_index', for some part of a `section`_

        Parameters
        ----------
        ifTemplatePart: Dict[:class:`str`, Any]
            The key-value pairs for some part in a `section`_

        Returns
        -------
        Any
            The corresponding value for the key 'match_first_index'
        """

        return ifTemplatePart[self.MatchFirstIndex]

    # _getAssetReplacement(assset, assetRepoAttName): Retrieves the replacement for 'asset'
    def _getAssetReplacement(self, asset: str, assetRepoAttName: str, modName: str) -> str:
        result = ""
        type = self.availableType

        if (type is not None):
            assetRepo = getattr(type, assetRepoAttName)
            result = assetRepo.replace(asset, version = self.version, toAssets = modName)
        else:
            raise NoModType()

        return result

    # _getHashReplacement(hash): Retrieves the hash replacement for 'hash'
    def _getHashReplacement(self, hash: str, modName: str) -> str:
        return self._getAssetReplacement(hash, "hashes", modName)
    
    # _getIndexReplacement(index): Retrieves the index replacement for 'index'
    def _getIndexReplacement(self, index: str, modName: str) -> str:
        return self._getAssetReplacement(index, "indices", modName)

    def _fillTextureOverrideRemapBlend(self, modName: str, sectionName: str, part: Dict[str, Any], partIndex: int, linePrefix: str, origSectionName: str) -> str:
        """
        Creates the **content part** of an :class:`IfTemplate` for the new sections created by this fix related to the ``[TextureOverride.*Blend.*]`` `sections`_

        .. note::
            For more info about an 'IfTemplate', see :class:`IfTemplate`

        Parameters
        ----------
        modName: :class:`str`
            The name for the type of mod to fix to

        sectionName: :class:`str`
            The new name for the section

        part: Dict[:class:`str`, Any]
            The content part of the :class:`IfTemplate` of the original [TextureOverrideBlend] `section`_

        partIndex: :class:`int`
            The index of where the content part appears in the :class:`IfTemplate` of the original `section`_

        linePrefix: :class:`str`
            The text to prefix every line of the created content part

        origSectionName: :class:`str`
            The name of the original `section`_

        Returns
        -------
        :class:`str`
            The created content part
        """

        addFix = ""

        for varName in part:
            varValue = part[varName]

            # filling in the subcommand
            if (varName == self.Run):
                subCommandName = self._getRemapName(varValue, modName, sectionGraph = self._blendCommandsGraph)
                subCommandStr = f"{self.Run} = {subCommandName}"
                addFix += f"{linePrefix}{subCommandStr}\n"

            # filling in the hash
            elif (varName == self.Hash):
                hash = self._getHashReplacement(varValue, modName)
                addFix += f"{linePrefix}{self.Hash} = {hash}\n"

            # filling in the vb1 resource
            elif (varName == self.Vb1):
                blendName = self._getIfTemplateResourceName(part)
                remapBlendName = self._getRemapName(blendName, modName, sectionGraph = self._resourceCommandsGraph, remapNameFunc = self.getRemapResourceName)
                fixStr = f'{self.Vb1} = {remapBlendName}'
                addFix += f"{linePrefix}{fixStr}\n"

            # filling in the handling
            elif (varName == self.Handling):
                fixStr = f'{self.Handling} = skip'
                addFix += f"{linePrefix}{fixStr}\n"

            # filling in the draw value
            elif (varName == self.Draw):
                fixStr = f'{self.Draw} = {varValue}'
                addFix += f"{linePrefix}{fixStr}\n"

            # filling in the indices
            elif (varName == self.MatchFirstIndex):
                index = self._getIndexReplacement(varValue, modName)
                addFix += f"{linePrefix}{self.MatchFirstIndex} = {index}\n"
                
        return addFix
    
    def _fillNonBlendSections(self, modName: str, sectionName: str, part: Dict[str, Any], partIndex: int, linePrefix: str, origSectionName: str) -> str:
        """
        Creates the **content part** of an :class:`IfTemplate` for the new sections created by this fix that are not related to the ``[TextureOverride.*Blend.*]`` `sections`_

        .. note::
            For more info about an 'IfTemplate', see :class:`IfTemplate`

        Parameters
        ----------
        modName: :class:`str`
            The name for the type of mod to fix to

        sectionName: :class:`str`
            The new name for the section

        part: Dict[:class:`str`, Any]
            The content part of the :class:`IfTemplate` of the original [TextureOverrideBlend] `section`_

        partIndex: :class:`int`
            The index of where the content part appears in the :class:`IfTemplate` of the original `section`_

        linePrefix: :class:`str`
            The text to prefix every line of the created content part

        origSectionName: :class:`str`
            The name of the original `section`_

        Returns
        -------
        :class:`str`
            The created content part
        """

        addFix = ""

        for varName in part:
            varValue = part[varName]

            # filling in the hash
            if (varName == self.Hash):
                newHash = self._getHashReplacement(varValue, modName)
                addFix += f"{linePrefix}{self.Hash} = {newHash}\n"

            # filling in the subcommand
            elif (varName == self.Run):
                subCommand = self._getRemapName(varValue, modName, sectionGraph = self._nonBlendHashIndexCommandsGraph, remapNameFunc = self.getRemapFixName)
                subCommandStr = f"{self.Run} = {subCommand}"
                addFix += f"{linePrefix}{subCommandStr}\n"

            # filling in the index
            elif (varName == self.MatchFirstIndex):
                newIndex = self._getIndexReplacement(varValue, modName)
                addFix += f"{linePrefix}{self.MatchFirstIndex} = {newIndex}\n"

            else:
                addFix += f"{linePrefix}{varName} = {varValue}\n"

        return addFix
    
    # fill the attributes for the sections related to the resources
    def _fillRemapResource(self, modName: str, sectionName: str, part: Dict[str, Any], partIndex: int, linePrefix: str, origSectionName: str):
        """
        Creates the **content part** of an :class:`IfTemplate` for the new `sections`_ created by this fix related to the ``[Resource.*Blend.*]`` `sections`_

        .. note::
            For more info about an 'IfTemplate', see :class:`IfTemplate`

        Parameters
        ----------
        modName: :class:`str`
            The name for the type of mod to fix to

        sectionName: :class:`str`
            The new name for the `section`_

        part: Dict[:class:`str`, Any]
            The content part of the :class:`IfTemplate` of the original ``[Resource.*Blend.*]`` `section`_

        partIndex: :class:`int`
            The index of where the content part appears in the :class:`IfTemplate` of the original `section`_

        linePrefix: :class:`str`
            The text to prefix every line of the created content part

        origSectionName: :class:`str`
            The name of the original `section`_

        Returns
        -------
        :class:`str`
            The created content part
        """

        addFix = ""

        for varName in part:
            varValue = part[varName]

            # filling in the subcommand
            if (varName == self.Run):
                subCommand = self._getRemapName(varValue, modName, sectionGraph = self._resourceCommandsGraph, remapNameFunc = self.getRemapResourceName)
                subCommandStr = f"{self.Run} = {subCommand}"
                addFix += f"{linePrefix}{subCommandStr}\n"

            # add in the type of file
            elif (varName == "type"):
                addFix += f"{linePrefix}type = Buffer\n"

            # add in the stride for the file
            elif (varName == "stride"):
                addFix += f"{linePrefix}stride = 32\n"

            # add in the file
            elif (varName == "filename"):
                remapModel = self.remapBlendModels[origSectionName]
                fixedBlendFile = remapModel.fixedBlendPaths[partIndex][modName]
                addFix += f"{linePrefix}filename = {fixedBlendFile}\n"

        return addFix
    
    # fills the if..else template in the .ini for each section
    def fillIfTemplate(self, modName: str, sectionName: str, ifTemplate: IfTemplate, fillFunc: Callable[[str, str, Union[str, Dict[str, Any]], int, int, str], str], origSectionName: Optional[str] = None) -> str:
        """
        Creates a new :class:`IfTemplate` for an existing `section`_ in the .ini file

        Parameters
        ----------
        modName: :class:`str`
            The name for the type of mod to fix to

        sectionName: :class:`str`
            The new name of the `section`_

        ifTemplate: :class:`IfTemplate`
            The :class:`IfTemplate` of the orginal `section`_

        fillFunc: Callable[[:class:`str`, :class:`str`, Union[:class:`str`, Dict[:class:`str`, Any], :class:`int`, :class:`str`, :class:`str`], :class:`str`]]
            The function to create a new **content part** for the new :class:`IfTemplate`
            :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                For more info about an 'IfTemplate', see :class:`IfTemplate`

            :raw-html:`<br />`
            The parameter order for the function is:

            #. The name for the type of mod to fix to
            #. The new section name
            #. The corresponding **content part** in the original :class:`IfTemplate`
            #. The index for the content part in the original :class:`IfTemplate`
            #. The string to prefix every line in the **content part** of the :class:`IfTemplate`
            #. The original name of the section

        origSectionName: Optional[:class:`str`]
            The original name of the section.

            If this argument is set to ``None``, then will assume this argument has the same value as the argument for ``sectionName`` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        :class:`str`
            The text for the newly created :class:`IfTemplate`
        """

        addFix = f"[{sectionName}]\n"
        partIndex = 0
        linePrefix = ""

        if (origSectionName is None):
            origSectionName = sectionName

        for part in ifTemplate:
            # adding in the if..else statements
            if (isinstance(part, str)):
                addFix += part
                
                linePrefix = re.match(r"^[( |\t)]*", part)
                if (linePrefix):
                    linePrefix = linePrefix.group(0)
                    linePrefixLen = len(linePrefix)

                    linePrefix = part[:linePrefixLen]
                    lStrippedPart = part[linePrefixLen:]

                    if (lStrippedPart.find("endif") == -1):
                        linePrefix += "\t"
                partIndex += 1
                continue
            
            # add in the content within the if..else statements
            addFix += fillFunc(modName, sectionName, part, partIndex, linePrefix, origSectionName)

            partIndex += 1
            
        return addFix
    

    # _getCommonMods(): Retrieves the common mods that need to be fixed between all target graphs
    #   that are used for the fix
    def _getCommonMods(self) -> Set[str]:
        if (self._type is None):
            return set()
        
        result = set()
        hashes = self._type.hashes
        indices = self._type.indices

        graphs = [self._blendCommandsGraph, self._nonBlendHashIndexCommandsGraph, self._resourceCommandsGraph]
        for graph in graphs:
            commonMods = graph.getCommonMods(hashes, indices, version = self.version)
            if (not result):
                result = commonMods
            else:
                result = result.intersection(commonMods)

        return result
    

    def _setToFix(self) -> Set[str]:
        """
        Sets the names for the types of mods that will used in the fix

        Returns
        -------
        Set[:class:`str`]
            The names of the mods that will be used in the fix        
        """

        commonMods = self._getCommonMods()
        toFix = commonMods.intersection(self.modsToFix)
        type = self.availableType

        if (not toFix and type is not None):
            self._toFix = type.getModsToFix()
        elif (not toFix):
            self._toFix = commonMods
        else:
            self._toFix = toFix

        return self._toFix

    
    # _makeRemapNames(): Makes the required names used for the fix
    def _makeRemapNames(self):
        self._blendCommandsGraph.getRemapBlendNames(self._toFix)
        self._nonBlendHashIndexCommandsGraph.getRemapBlendNames(self._toFix)
        self._resourceCommandsGraph.getRemapBlendNames(self._toFix)


    # _getRemapName(sectionName, modName, sectionGraph, remapNameFunc): Retrieves the required remap name for the fix
    def _getRemapName(self, sectionName: str, modName: str, sectionGraph: Optional[IniSectionGraph] = None, remapNameFunc: Optional[Callable[[str, str], str]] = None) -> str:
        error = False
        if (sectionGraph is None):
            error = True

        if (not error):
            try:
                return sectionGraph.remapNames[sectionName][modName]
            except KeyError:
                error = True

        if (remapNameFunc is None):
            remapNameFunc = self.getRemapBlendName

        return remapNameFunc(sectionName, modName)


    def getModFixStr(self, modName: str, fix: str = ""):
        """
        Generates the newly added code in the .ini file for the fix of a single type of mod

        .. note::
            eg.
                If we are making the fix from ``Jean`` -> ``JeanCN`` and ``JeanSeaBreeze``,
                The code below will only make the fix for ``JeanCN``

            .. code-block::
                .. code-block::

                getModFixStr("JeanCN")


        Parameters
        ----------
        fix: :class:`str`
            Any existing text we want the result of the fix to add onto :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ""

        Returns
        -------
        :class:`str`
            The text for the newly generated code in the .ini file
        """

        hasNonBlendSections = bool(self._nonBlendHashIndexCommandsGraph.sections)
        hasResources = bool(self.remapBlendModels)

        if (self._blendCommandsGraph.sections or hasResources or hasNonBlendSections):
            fix += "\n"

        # get the fix string for all the texture override blends
        blendCommandTuples = self._blendCommandsGraph.runSequence
        for commandTuple in blendCommandTuples:
            section = commandTuple[0]
            ifTemplate = commandTuple[1]
            commandName = self._getRemapName(section, modName, sectionGraph = self._blendCommandsGraph)
            fix += self.fillIfTemplate(modName, commandName, ifTemplate, self._fillTextureOverrideRemapBlend)
            fix += "\n"

        if (hasNonBlendSections):
            fix += "\n"

        # get the fix string for non-blend sections
        nonBlendCommandTuples = self._nonBlendHashIndexCommandsGraph.runSequence
        for commandTuple in nonBlendCommandTuples:
            section = commandTuple[0]
            ifTemplate = commandTuple[1]
            commandName = self._getRemapName(section, modName, sectionGraph = self._nonBlendHashIndexCommandsGraph)
            fix += self.fillIfTemplate(modName, commandName, ifTemplate, self._fillNonBlendSections)
            fix += "\n"

        if (hasResources):
            fix += "\n"

        # get the fix string for the resources
        resourceCommandTuples = self._resourceCommandsGraph.runSequence
        resourceCommandsLen = len(resourceCommandTuples)
        for i in range(resourceCommandsLen):
            commandTuple = resourceCommandTuples[i]
            section = commandTuple[0]
            ifTemplate = commandTuple[1]

            resourceName = self._getRemapName(section, modName, sectionGraph = self._resourceCommandsGraph, remapNameFunc = self.getRemapResourceName)
            fix += self.fillIfTemplate(modName, resourceName, ifTemplate, self._fillRemapResource, origSectionName = section)

            if (i < resourceCommandsLen - 1):
                fix += "\n"

        return fix


    # get the needed lines to fix the .ini file
    @_addFixBoilerPlate
    def getFixStr(self, fix: str = "") -> str:
        """
        Generates the newly added code in the .ini file for the fix

        Parameters
        ----------
        fix: :class:`str`
            Any existing text we want the result of the fix to add onto :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ""

        Returns
        -------
        :class:`str`
            The text for the newly generated code in the .ini file
        """

        heading = Heading("", sideLen = 5, sideChar = "*")
        for modName in self._toFix:
            heading.title = modName
            currentFix = self.getModFixStr(modName)

            if (currentFix):
                fix += f"\n\n; {heading.open()}{currentFix}"

        return fix


    @_readLines
    def injectAddition(self, addition: str, beforeOriginal: bool = True, keepBackup: bool = True, fixOnly: bool = False) -> str:
        """
        Adds and writes new text to the .ini file

        Parameters
        ----------
        addition: :class:`str`
            The text we want to add to the file

        beforeOriginal: :class:`bool`
            Whether to add the new text before the original text :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        keepBackup: :class:`bool`
            Whether we want to make a backup copy of the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        fixOnly: :class:`bool`
            Whether we are only fixing the .ini file without removing any previous changes :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`str`
            The content of the .ini file with the new text added
        """

        original = "".join(self._fileLines)

        if (keepBackup and fixOnly and self._filePath is not None):
            self.print("log", "Cleaning up and disabling the OLD STINKY ini")
            self.disIni()

        result = ""
        if (beforeOriginal):
            result = f"{addition}\n\n{original}"
        else:
            result = f"{original}\n{addition}"

        # writing the fixed file
        if (self._filePath is not None):
            with open(self._filePath.path, "w", encoding = IniFileEncoding) as f:
                f.write(result)

        self._isFixed = True
        return result
    
    # _getRemovalResource(sectionsToRemove): Retrieves the names of the resource sections to remove
    def _getRemovalResource(self, sectionsToRemove: Set[str]) -> Set[str]:
        result = set()
        allSections = self.getIfTemplates()
        removalSectionGraph = IniSectionGraph(sectionsToRemove, allSections)
        self._getBlendResources(result, removalSectionGraph)

        result = set(filter(lambda section: re.match(self._sectionRemovalPattern, section), result))
        return result

    @_readLines
    def _removeScriptFix(self, parse: bool = False) -> str:
        """
        Removes the dedicated section of the code in the .ini file that this script has made

        Parameters
        ----------
        parse: :class:`bool`
            Whether to keep track of the Blend.buf files that also need to be removed :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`str`
            The new text content of the .ini file
        """

        if (not parse):
            self._fileTxt = re.sub(self._fixRemovalPattern, "", self._fileTxt)
        else:
            removedSectionsIndices = []
            txtLinesToRemove = []

            # retrieve the indices the dedicated section is located
            rangesToRemove = [match.span() for match in re.finditer(self._fixRemovalPattern, self._fileTxt)]
            for range in rangesToRemove:
                start = range[0]
                end = range[1]
                txtLines = TextTools.getTextLines(self._fileTxt[start : end])

                removedSectionsIndices.append(range)
                txtLinesToRemove += txtLines

            # retrieve the names of the sections the dedicated sections reference
            sectionNames = set()
            for line in txtLinesToRemove:
                if (re.match(self._sectionPattern, line)):
                    sectionName = self._getSectionName(line)
                    sectionNames.add(sectionName)

            resourceSections = self._getRemovalResource(sectionNames)

            # get the Blend.buf files that need to be removed
            self._makeRemovalRemapModels(resourceSections)
            
            # remove the dedicated section
            self._fileTxt = TextTools.removeParts(self._fileTxt, removedSectionsIndices)

        self.fileTxt = self._fileTxt.strip()
        result = self.write()

        self.clearRead()
        self._isFixed = False
        return result
    
    @_readLines
    def _removeFixSections(self, parse: bool = False) -> str:
        """
        Removes the [.*RemapBlend.*] sections of the .ini file that this script has made

        Parameters
        ----------
        parse: :class:`bool`
            Whether to keep track of the Blend.buf files that also need to be removed :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`str`
            The new text content of the .ini file
        """

        if (not parse):
            self.removeSectionOptions(self._removalPattern)
        else:
            sectionsToRemove = self.getSectionOptions(self._removalPattern, postProcessor = self._removeSection)

            sectionNames = set()
            removedSectionIndices = []

            # get the indices and sections to remove
            for sectionName in sectionsToRemove:
                sectionRanges = sectionsToRemove[sectionName]
                sectionNames.add(sectionName)

                for range in sectionRanges:
                    removedSectionIndices.append(range)

            resourceSections = self._getRemovalResource(sectionNames)
            self._makeRemovalRemapModels(resourceSections)
            self.fileLines = TextTools.removeLines(self.fileLines, removedSectionIndices)

        result = self.write()
        self._removedSectionsIndices = None

        self.clearRead()
        self._isFixed = False
        return result

    def _removeFix(self, parse: bool = False) -> str:
        """
        Removes any previous changes that were probably made by this script :raw-html:`<br />` :raw-html:`<br />`

        For the .ini file will remove:

        #. All code surrounded by the *'---...--- .* Fix ---...----'* header/footer
        #. All `sections`_ containing the keywords ``RemapBlend``

        Parameters
        ----------
        parse: :class:`bool`
            Whether to keep track of the Blend.buf files that also need to be removed :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`str`
            The new text content of the .ini file with the changes removed
        """

        self._removeScriptFix(parse = parse)    
        result = self._removeFixSections(parse = parse)
        return result

    @_readLines
    def removeFix(self, keepBackups: bool = True, fixOnly: bool = False, parse: bool = False) -> str:
        """
        Removes any previous changes that were probably made by this script and creates backup copies of the .ini file

        .. note::
            For more info about what gets removed from the .ini file, see :meth:`IniFile._removeFix`

        Parameters
        ----------
        keepBackup: :class:`bool`
            Whether we want to make a backup copy of the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        fixOnly: :class:`bool`
            Whether we are only fixing the .ini file without removing any previous changes :raw-html:`<br />` :raw-html:`<br />`

            .. note::
                If this value is set to ``True``, then the previous changes made by this script will not be removed

            **Default**: ``False``

        parse: :class:`bool`
            Whether to also parse for the .*RemapBlend.buf files that need to be removed

        Returns
        -------
        :class:`str`
            The new text content of the .ini file with the changes removed
        """

        if (keepBackups and not fixOnly and self._filePath is not None):
            self.print("log", f"Creating Backup for {self._filePath.base}")
            self.disIni(makeCopy = True)

        if (fixOnly):
            return self._fileTxt

        if (self._filePath is not None):
            self.print("log", f"Removing any previous changes from this script in {self._filePath.base}")

        result = self._removeFix(parse = parse)
        return result
    
    def _makeRemapModel(self, ifTemplate: IfTemplate, toFix: Optional[Set[str]] = None, getFixedFile: Optional[Callable[[str], str]] = None) -> RemapBlendModel:
        """
        Creates the data needed for fixing a particular ``[Resource.*Blend.*]`` `section`_ in the .ini file

        Parameters
        ----------
        ifTemplate: :class:`IfTemplate`
            The particular `section`_ to extract data

        toFix: Optional[Set[:class:`str`]]
            The names of the mods to fix :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will used the names of the mods to fix from this class :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        getFixedFile: Optional[Callable[[:class:`str`, :class:`str`], :class:`str`]]
            The function for transforming the file path of a found .*Blend.buf file into a .*RemapBlend.buf file :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will use :meth:`IniFile.getFixedBlendFile` :raw-html:`<br />` :raw-html:`<br />`

            The parameters for the function are:

                # The path to the original file
                # The type of mod to fix to

            **Default**: ``None``

        Returns
        -------
        :class:`RemapBlendModel`
            The data for fixing the particular 
        """

        folderPath = self.folder

        if (toFix is None):
            toFix = self._toFix

        if (getFixedFile is None):
            getFixedFile = self.getFixedBlendFile

        origBlendPaths = {}
        fixedBlendPaths = {}
        partIndex = 0

        for part in ifTemplate:
            if (isinstance(part,str)):
                partIndex += 1
                continue

            origBlendFile = None
            try:
                origBlendFile = FileService.parseOSPath(part['filename'])
            except KeyError:
                partIndex += 1
                continue
            
            origBlendPaths[partIndex] = origBlendFile

            for modName in toFix:
                fixedBlendPath = getFixedFile(origBlendFile, modName = modName)
                try:
                    fixedBlendPaths[partIndex]
                except KeyError:
                    fixedBlendPaths[partIndex] = {}

                fixedBlendPaths[partIndex][modName] = fixedBlendPath

            partIndex += 1

        remapBlendModel = RemapBlendModel(folderPath, fixedBlendPaths, origBlendPaths = origBlendPaths)
        return remapBlendModel


    #_makeRemovalRemapModels(sectionNames): Retrieves the data needed for removing Blend.buf files from the .ini file
    def _makeRemovalRemapModels(self, sectionNames: Set[str]):
        for sectionName in sectionNames:
            ifTemplate = None
            try:
                ifTemplate = self._sectionIfTemplates[sectionName]
            except:
                continue

            self.remapBlendModels[sectionName] = self._makeRemapModel(ifTemplate, toFix = {""}, getFixedFile = lambda origFile, modName: origFile)


    def _makeRemapModels(self, resourceGraph: IniSectionGraph, toFix: Optional[Set[str]] = None, getFixedFile: Optional[Callable[[str], str]] = None) -> Dict[str, RemapBlendModel]:
        """
        Creates all the data needed for fixing the ``[Resource.*Blend.*]`` `sections`_ in the .ini file

        Parameters
        ----------
        resourceGraph: :class:`IniSectionGraph`
            The graph of `sections`_ for the resources

        toFix: Optional[Set[:class:`str`]]
            The names of the mods to fix :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will used the names of the mods to fix from this class :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        getFixedFile: Optional[Callable[[:class:`str`], :class:`str`]]
            The function for transforming the file path of a found .*Blend.buf file into a .*RemapBlend.buf file :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will use :meth:`IniFile.getFixedBlendFile` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Returns
        -------
        Dict[:class:`str`, :class:`RemapBlendModel`]
            The data for fixing the resource `sections`_

            The keys are the original names for the resource `sections`_ and the values are the required data for fixing the `sections`_
        """

        resourceCommands = resourceGraph.sections
        for resourceKey in resourceCommands:
            resourceIftemplate = resourceCommands[resourceKey]
            remapBlendModel = self._makeRemapModel(resourceIftemplate, toFix = toFix, getFixedFile = getFixedFile)
            self.remapBlendModels[resourceKey] = remapBlendModel

        return self.remapBlendModels

    def _getSubCommands(self, ifTemplate: IfTemplate, currentSubCommands: Set[str], subCommands: Set[str], subCommandLst: List[str]):
        for partIndex in ifTemplate.calledSubCommands:
            subCommand = ifTemplate.calledSubCommands[partIndex]
            if (subCommand not in subCommands):
                currentSubCommands.add(subCommand)
                subCommands.add(subCommand)
                subCommandLst.append(subCommand)

    def _getCommandIfTemplate(self, sectionName: str, raiseException: bool = True) -> Optional[IfTemplate]:
        """
        Retrieves the :class:`IfTemplate` for a certain `section`_ from `IniFile._sectionIfTemplate`

        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_

        raiseException: :class:`bool`
            Whether to raise an exception when the section's :class:`IfTemplate` is not found

        Raises
        ------
        :class:`KeyError`
            If the :class:`IfTemplate` for the `section`_ is not found and ``raiseException`` is set to `True`

        Returns
        -------
        Optional[:class:`IfTemplate`]
            The corresponding :class:`IfTemplate` for the `section`_
        """
        try:
            ifTemplate = self._sectionIfTemplates[sectionName]
        except Exception as e:
            if (raiseException):
                raise KeyError(f"The section by the name '{sectionName}' does not exist") from e
            else:
                return None
        else:
            return ifTemplate

    def _getBlendResources(self, blendResources: Set[str], blendCommandsGraph: IniSectionGraph):
        """
        Retrieves all the referenced resources that were called by `sections`_ related to the ``[TextureOverride.*Blend.*]`` `sections`_

        Parameters
        ----------
        blendResources: Set[:class:`str`]
            The result for all the resource `sections`_ that were referenced

        blendCommandsGraph: :class:`IniSectionGraph`
            The subgraph for all the `sections`_ related to the ``[TextureOverride.*Blend.*]`` `sections`_
        """

        blendSections = blendCommandsGraph.sections
        for sectionName in blendSections:
            ifTemplate = blendSections[sectionName]

            for part in ifTemplate:
                if (isinstance(part, str)):
                    continue

                if (self._isIfTemplateResource(part)):
                    resource = self._getIfTemplateResourceName(part)
                    blendResources.add(resource)

    def _getCommands(self, sectionName: str, subCommands: Set[str], subCommandLst: List[str]):
        """
        Low level function for retrieving all the commands/`sections`_ that are called from a certain `section`_ in the .ini file

        Parameters
        ----------
        sectionName: :class:`str`
            The name of the `section`_ we are starting from

        subCommands: Set[:class:`str`]
            The result for all of the `sections`_ that were called

        subCommandLst: List[:class:`str`]
            The result for all of the `sections`_ that were called while maintaining the order
            the `sections`_ are called in the call stack

        Raises
        ------
        :class:`KeyError`
            If the :class:`IfTemplate` is not found for some `section`_
        """

        currentSubCommands = set()
        ifTemplate = self._getCommandIfTemplate(sectionName)

        # add in the current command if it has not been added yet
        if (sectionName not in subCommands):
            subCommands.add(sectionName)
            subCommandLst.append(sectionName)

        # get all the unvisited subcommand sections to visit
        self._getSubCommands(ifTemplate, currentSubCommands, subCommands, subCommandLst)

        # visit the children subcommands that have not been visited yet
        for sectionName in currentSubCommands:
            self._getCommands(sectionName, subCommands, subCommandLst)


    # _getTargetHashAndIndexSections(blendCommandNames): Retrieves the sections with target hashes and indices
    def _getTargetHashAndIndexSections(self, blendCommandNames: Set[str]) -> Set[IfTemplate]:
        if (self._type is None and self.defaultModType is None):
            return set()
        
        type = self._type
        if (self._type is None):
            type = self.defaultModType

        result = {}
        hashes = set(type.hashes.fromAssets)
        indices = set(type.indices.fromAssets)
        
        # get the sections with the hashes/indices
        for sectionName in self._sectionIfTemplates:
            ifTemplate = self._sectionIfTemplates[sectionName]
            if (sectionName in blendCommandNames):
                continue

            if (hashes.intersection(ifTemplate.hashes) or indices.intersection(ifTemplate.indices)):
                result[sectionName] = ifTemplate

        return result

    # parse(): Parses the merged.ini file for any info needing to keep track of
    def parse(self):
        """
        Parses the .ini file

        Raises
        ------
        :class:`KeyError`
            If a certain resource `section`_ is not found :raw-html:`<br />` :raw-html:`<br />`
            
            (either the name of the `section`_ is not found in the .ini file or the `section`_ was skipped due to some error when parsing the `section`_)
        """

        self._blendCommandsGraph.build(newTargetSections = [], newAllSections = {})
        self._resourceCommandsGraph.build(newTargetSections = [], newAllSections = {})
        self.remapBlendModels = {}

        self.getIfTemplates(flush = True)
        if (self.defaultModType is not None and self._textureOverrideBlendSectionName is not None and self._textureOverrideBlendRoot is None):
            self._textureOverrideBlendRoot = self._textureOverrideBlendSectionName

        try:
            self._sectionIfTemplates[self._textureOverrideBlendRoot]
        except:
            return

        blendResources = set()

        # build the blend commands DFS forest
        subCommands = { self._textureOverrideBlendRoot }
        self._blendCommandsGraph.build(newTargetSections = subCommands, newAllSections = self._sectionIfTemplates)

        # build the DFS forest for the other sections that contain target hashes/indices that are not part of the blend commands
        hashIndexSections = self._getTargetHashAndIndexSections(set(self._blendCommandsGraph.sections.keys()))
        self._nonBlendHashIndexCommandsGraph.build(newTargetSections = hashIndexSections, newAllSections= self._sectionIfTemplates)

        # keep track of all the needed blend dependencies
        self._getBlendResources(blendResources, self._blendCommandsGraph)

        # sort the resources
        resourceCommandLst = list(map(lambda resourceName: (resourceName, self.getMergedResourceIndex(resourceName)), blendResources))
        resourceCommandLst.sort(key = cmp_to_key(self._compareResources))
        resourceCommandLst = list(map(lambda resourceTuple: resourceTuple[0], resourceCommandLst))

        # keep track of all the subcommands that the resources call
        self._resourceCommandsGraph.build(newTargetSections = resourceCommandLst, newAllSections = self._sectionIfTemplates)

        # get the required files that need fixing
        self._setToFix()
        self._makeRemapNames()
        self._makeRemapModels(self._resourceCommandsGraph)

    def fix(self, keepBackup: bool = True, fixOnly: bool = False) -> str:
        """
        Fixes the .ini file

        Parameters
        ----------
        keepBackup: :class:`bool`
            Whether we want to make a backup copy of the .ini file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: `True`

        fixOnly: :class:`bool`
            Whether we are only fixing the .ini file without removing any previous changes :raw-html:`<br />` :raw-html:`<br />`

            **Default**: `False`

        Returns
        -------
        :class:`str`
            The new content of the .ini file which includes the fix
        """

        fix = ""
        fix += self.getFixStr(fix = fix)
        result = self.injectAddition(f"\n\n{fix}", beforeOriginal = False, keepBackup = keepBackup, fixOnly = fixOnly)
        self._isFixed = True
        return result
    

class BlendFile(Model):
    """
    This Class inherits from :class:`Model`

    Used for handling blend.buf files

    .. note::
        We observe that a Blend.buf file is a binary file defined as:

        * each line contains 32 bytes (256 bits)
        * each line uses little-endian mode (MSB is to the right while LSB is to the left)
        * the first 16 bytes of a line are for the blend weights, each weight is 4 bytes or 32 bits (4 weights/line)
        * the last 16 bytes of a line are for the corresponding indices for the blend weights, each index is 4 bytes or 32 bits (4 indices/line)
        * the blend weights are floating points while the blend indices are unsigned integers

    Parameters
    ----------
    src: Union[:class:`str`, :class:`bytes`]
        The source file or bytes for the blend file

    Attributes
    ----------
    src: Union[:class:`str`, :class:`bytes`]
        The source file or bytes for the blend file

    _data: :class:`bytes`
        The bytes read from the source
    """

    BytesPerLine = 32

    def __init__(self, src: Union[str, bytes]):
        self.src = src
        self._data = self.read()

    def read(self) -> bytes:
        """
        Reads the bytes in the blend.buf file

        Returns
        -------
        :class:`bytes`
            The read bytes
        """

        return self.readFile(self.src)

    @classmethod
    def readFile(cls, blendSrc: Union[str, bytes]):
        result = FileService.readBinary(blendSrc)
        isValid = cls._isValid(result)

        if (not isValid and isinstance(blendSrc, str)):
            raise BlendFileNotRecognized(blendSrc)
        elif (not isValid):
            raise BadBlendData()
        
        return result

    @classmethod
    def _getLineWeight(cls, data: bytes, lineInd: int) -> Tuple[float, float, float, float]:
        return [struct.unpack("<f", data[lineInd + 4 * j : lineInd + 4 * (j+1)])[0] for j in range(4)]
    
    @classmethod
    def _getLineIndices(cls, data: bytes, lineInd: int) -> Tuple[int, int, int, int]:
        return [struct.unpack("<I", data[lineInd + 16 + 4 * j : lineInd + 16 + 4 * (j+1)])[0] for j in range(4)]

    @classmethod
    def _isValid(cls, data: bytes):
        if (len(data) % cls.BytesPerLine != 0):
            return False
        return True

    def correct(self, vgRemap: VGRemap, fixedBlendFile: Optional[str] = None) -> Union[Optional[str], bytearray]:
        """
        Fixes a Blend.buf file

        Parameters
        ----------
        vgRemap: :class:`VGRemap`
            The vertex group remap for correcting the Blend.buf file

        fixedBlendFile: Optional[:class:`str`]
            The file path for the fixed Blend.buf file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Raises
        ------
        :class:`BlendFileNotRecognized`
            If the original Blend.buf file provided by the parameter ``blendFile`` cannot be read

        :class:`BadBlendData`
            If the bytes passed into this function do not correspond to the format defined for a Blend.buf file

        Returns
        -------
        Union[Optional[:class:`str`], :class:`bytearray`]
            If the argument ``fixedBlendFile`` is ``None``, then will return an array of bytes for the fixed Blend.buf file :raw-html:`<br />` :raw-html:`<br />`
            Otherwise will return the filename to the fixed RemapBlend.buf file if the provided Blend.buf file got corrected
        """

        # if no correction is needed to be done
        blendFile = self.src
        blendIsFile = isinstance(blendFile, str)
        if (not vgRemap.remap and blendIsFile):
            return None
        elif (not vgRemap.remap):
            return bytearray(blendFile)

        result = bytearray()
        dataLen = len(self._data)
        for i in range(0,dataLen,32):
            blendweights = self._getLineWeight(self._data, i)
            blendindices = self._getLineIndices(self._data, i)
            outputweights = bytearray()
            outputindices = bytearray()

            # replaces the blend index in the original mod with the corresponding blend index
            #   for the boss
            for weight, index in zip(blendweights, blendindices):
                if weight != 0 and index <= vgRemap.maxIndex:
                    index = int(vgRemap.remap[index])
                outputweights += struct.pack("<f", weight)
                outputindices += struct.pack("<I", index)
            result += outputweights
            result += outputindices

        if (fixedBlendFile is not None):
            FileService.writeBinary(fixedBlendFile, result)
            return fixedBlendFile

        return result

    @classmethod
    def _addRemap(cls, hasRemap: bool, remap: Dict[bytes, Union[bytes, List[bytes]]], key: bytes, value: bytes) -> bool:
        currentIsRemap = True
        try:
            remap[key]
        except KeyError:
            remap[key] = value
        else:
            remapValue = remap[key]

            if (remapValue != value):
                currentIsRemap = False

                if (not isinstance(remapValue, list)):
                    remap[key] = [remapValue]

                remap[key].append(value)

        return (hasRemap and currentIsRemap)


class Mod(Model):
    """
    This Class inherits from :class:`Model`

    Used for handling a mod

    .. note::
        We define **a mod** based off the following criteria:

        * A folder that contains at least 1 .ini file
        * At least 1 of the .ini files in the folder contains:

            * a section with the regex ``[TextureOverride.*Blend]`` if :attr:`RemapService.readAllInis` is set to ``True`` or the script is ran with the ``--all`` flag :raw-html:`<br />`  :raw-html:`<br />` **OR** :raw-html:`<br />` :raw-html:`<br />`
            * a section that meets the criteria of one of the mod types defined :attr:`Mod._types` by running the mod types' :meth:`ModType.isType` function

        :raw-html:`<br />`
        See :class:`ModTypes` for some predefined types of mods
        
    Parameters
    ----------
    path: Optional[:class:`str`]
        The file location to the mod folder. :raw-html:`<br />` :raw-html:`<br />`
        
        If this value is set to ``None``, then will use the current directory of where this module is loaded.
        :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    files: Optional[List[:class:`str`]]
        The direct children files to the mod folder (does not include files located in a folder within the mod folder). :raw-html:`<br />` :raw-html:`<br />`

        If this parameter is set to ``None``, then the class will search the files for you when the class initializes :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    logger: Optional[:class:`Logger`]
        The logger used to pretty print messages :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    types: Optional[Set[:class:`ModType`]]
        The types of mods this mod should be. :raw-html:`<br />` :raw-html:`<br />` 
        If this argument is empty or is ``None``, then all the .ini files in this mod will be parsed :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    defaultType: Optional[:class:`ModType`]
        The type of mod to use if a mod has an unidentified type :raw-html:`<br />` :raw-html:`<br />`
        If this argument is ``None``, then will skip the mod with an identified type :raw-html:`<br />` :raw-html:`<br />` 

        **Default**: ``None``

    version: Optional[:class:`float`]
        The game version we want the fixed mod :raw-html:`<br />` :raw-html:`<br />`

        If This value is ``None``, then will fix the mod to using the latest hashes/indices.

    Attributes
    ----------
    path: Optional[:class:`str`]
        The file location to the mod folder

    version: Optional[:class:`float`]
        The game version we want the fixed mod

    _files: List[:class:`str`]
        The direct children files to the mod folder (does not include files located in a folder within the mod folder).

    _types: Set[:class:`ModType`]
        The types of mods this mod should be

    _defaultType: Optional[:class:`ModType`]
        The type of mod to use if a mod has an unidentified type

    logger: Optional[:class:`Logger`]
        The logger used to pretty print messages

    inis: List[:class:`str`]
        The .ini files found for the mod

    remapBlend: List[:class:`str`]
        The RemapBlend.buf files found for the mod

    backupInis: List[:class:`str`]
        The DISABLED_BossFixBackup.txt files found for the mod

    backupDups: List[:class:`str`]
        The DISABLED_RSDup.txt files found for the mod

        .. warning::
            This attribute is now DEPRECATED. Now, the fix does not care whether there are duplicate .ini files or Blend.buf files
    """
    def __init__(self, path: Optional[str] = None, files: Optional[List[str]] = None, logger: Optional[Logger] = None, types: Optional[Set[ModType]] = None, defaultType: Optional[ModType] = None, version: Optional[float] = None):
        super().__init__(logger = logger)
        self.path = FileService.getPath(path)
        self.version = version
        self._files = files
        if (types is None):
            types = set()
        self._types = types
        self._defaultType = defaultType

        self.inis = []
        self.remapBlend = []
        self.backupInis = []
        self._setupFiles()

    @property
    def files(self):
        """
        The direct children files to the mod folder (does not include files located in a folder within the mod folder).

        :getter: Returns the files to the mod
        :setter: Sets up the files for the mod
        :type: Optional[List[:class:`str`]]
        """

        return self._files

    @files.setter
    def files(self, newFiles: Optional[List[str]] = None):
        self._files = newFiles
        self._setupFiles()

    def _setupFiles(self):
        """
        Searches the direct children files to the mod folder if :attr:`Mod.files` is set to ``None``        
        """

        if (self._files is None):
            self._files = FileService.getFiles(path = self.path)

        self.inis, self.remapBlend, self.backupInis = self.getOptionalFiles()
        self.inis = list(map(lambda iniPath: IniFile(iniPath, logger = self.logger, modTypes = self._types, defaultModType = self._defaultType, version = self.version), self.inis))

    @classmethod
    def isIni(cls, file: str) -> bool:
        """
        Determines whether the file is a .ini file which is the file used to control how a mod behaves

        Parameters
        ----------
        file: :class:`str`
            The file path to check

        Returns
        -------
        :class:`bool`
            Whether the passed in file is a .ini file
        """

        return file.endswith(FileExt.Ini.value)
    
    @classmethod
    def isRemapBlend(cls, file: str) -> bool:
        """
        Determines whether the file is a RemapBlend.buf file which is the fixed Blend.buf file created by this fix

        Parameters
        ----------
        file: :class:`str`
            The file path to check

        Returns
        -------
        :class:`bool`
            Whether the passed in file is a RemapBlend.buf file
        """

        baseName = os.path.basename(file)
        if (not baseName.endswith(FileExt.Buf.value)):
            return False

        baseName = baseName.rsplit(".", 1)[0]
        baseNameParts = baseName.rsplit("RemapBlend", 1)

        return (len(baseNameParts) > 1)
    
    @classmethod
    def isBlend(cls, file: str) -> bool:
        """
        Determines whether the file is a Blend.buf file which is the original blend file provided in the mod

        Parameters
        ----------
        file: :class:`str`
            The file path to check

        Returns
        -------
        :class:`bool`
            Whether the passed in file is a Blend.buf file
        """

        return bool(file.endswith(FileTypes.Blend.value) and not cls.isRemapBlend(file))
   
    @classmethod
    def isBackupIni(cls, file: str) -> bool:
        """
        Determines whether the file is a DISABLED_BossFixBackup.txt file that is used to make
        backup copies of .ini files

        Parameters
        ----------
        file: :class:`str`
            The file path to check

        Returns
        -------
        :class:`bool`
            Whether the passed in file is a DISABLED_BossFixBackup.txt file
        """

        fileBaseName = os.path.basename(file)
        return (fileBaseName.startswith(BackupFilePrefix) or fileBaseName.startswith(OldBackupFilePrefix)) and file.endswith(FileExt.Txt.value)

    def getOptionalFiles(self) -> List[Optional[str]]:
        """
        Retrieves a list of each type of files that are not mandatory for the mod

        Returns
        -------
        [ List[:class:`str`], List[:class:`str`], List[:class:`str`]]
            The resultant files found for the following file categories (listed in the same order as the return type):

            #. .ini files
            #. .RemapBlend.buf files
            #. DISABLED_BossFixBackup.txt files

            .. note::
                See :meth:`Mod.isIni`, :meth:`Mod.isRemapBlend`, :meth:`Mod.isBackupIni` for the specifics of each type of file
        """

        SingleFileFilters = {}
        MultiFileFilters = [self.isIni, self.isRemapBlend, self.isBackupIni]

        singleFiles = []
        if (SingleFileFilters):
            singleFiles = FileService.getSingleFiles(path = self.path, filters = SingleFileFilters, files = self._files, optional = True)
        multiFiles = FileService.getFiles(path = self.path, filters = MultiFileFilters, files = self._files)

        result = singleFiles
        if (not isinstance(result, list)):
            result = [result]

        result += multiFiles
        return result
    
    def removeBackupInis(self):
        """
        Removes all DISABLED_BossFixBackup.txt contained in the mod
        """

        for file in self.backupInis:
            self.print("log", f"Removing the backup ini, {os.path.basename(file)}")
            os.remove(file)

    def removeFix(self, fixedBlends: Set[str], fixedInis: Set[str], visitedRemapBlendsAtRemoval: Set[str], inisSkipped: Dict[str, Exception], keepBackups: bool = True, fixOnly: bool = False) -> List[Set[str]]:
        """
        Removes any previous changes done by this module's fix

        Parameters
        ----------
        fixedBlend: Set[:class:`str`]
            The file paths to the RemapBlend.buf files that we do not want to remove

        fixedInis: Set[:class:`str`]
            The file paths to the .ini files that we do not want to remove

        visitedRemapBlendsAtRemoval: Set[:class:`str`]
            The file paths to the RemapBlend.buf that have already been attempted to be removed

        inisSkipped: Dict[:class:`str`, :class:`Exception`]
            The file paths to the .ini files that are skipped due to errors

        keepBackups: :class:`bool`
            Whether to create or keep DISABLED_BossFixBackup.txt files in the mod :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        fixOnly: :class:`bool`
            Whether to not undo any changes created in the .ini files :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        [Set[:class:`str`], Set[:class:`str`]]
            The removed files that have their fix removed, where the types of files for the return value is based on the list below:

            #. .ini files with their fix removed
            #. RemapBlend.buf files that got deleted
        """

        removedRemapBlends = set()
        undoedInis = set()

        for ini in self.inis:
            remapBlendsRemoved = False
            iniFilesUndoed = False
            iniFullPath = None
            iniHasErrors = False
            if (ini.file is not None):
                iniFullPath = FileService.absPathOfRelPath(ini.file, self.path)

            # parse the .ini file even if we are only undoing fixes for the case where a Blend.buf file
            #   forms a bridge with some disconnected folder subtree of a mod
            # Also, we only want to remove the Blend.buf files connected to particular types of .ini files, 
            #   instead of all the Blend.buf files in the folder
            if (iniFullPath is None or (iniFullPath not in fixedInis and iniFullPath not in inisSkipped)):
                try:
                    ini.parse()
                except Exception as e:
                    inisSkipped[iniFullPath] = e
                    iniHasErrors = True
                    self.print("handleException", e)

            # remove the fix from the .ini files
            if (not iniHasErrors and iniFullPath is not None and iniFullPath not in fixedInis and iniFullPath not in inisSkipped and ini.isModIni):
                try:
                    ini.removeFix(keepBackups = keepBackups, fixOnly = fixOnly, parse = True)
                except Exception as e:
                    inisSkipped[iniFullPath] = e
                    iniHasErrors = True
                    self.print("handleException", e)
                    continue

                undoedInis.add(iniFullPath)

                if (not iniFilesUndoed):
                    iniFilesUndoed = True

            if (iniFilesUndoed):
                self.print("space")

            # remove only the remap blends that have not been recently created
            for _, blendModel in ini.remapBlendModels.items():
                for partIndex, partFullPaths in blendModel.fullPaths.items():
                    for modName in partFullPaths:
                        remapBlendFullPath = partFullPaths[modName]

                        if (remapBlendFullPath not in fixedBlends and remapBlendFullPath not in visitedRemapBlendsAtRemoval):
                            try:
                                os.remove(remapBlendFullPath)
                            except FileNotFoundError as e:
                                self.print("log", f"No Previous {RemapBlendFile} found at {remapBlendFullPath}")
                            else:
                                self.print("log", f"Removing previous {RemapBlendFile} at {remapBlendFullPath}")
                                removedRemapBlends.add(remapBlendFullPath)

                            visitedRemapBlendsAtRemoval.add(remapBlendFullPath)
                            if (not remapBlendsRemoved):
                                remapBlendsRemoved = True

            if (remapBlendsRemoved):
                self.print("space")

        return [undoedInis, removedRemapBlends]

    @classmethod
    def blendCorrection(cls, blendFile: Union[str, bytes], modType: ModType, modToFix: str, 
                        fixedBlendFile: Optional[str] = None, version: Optional[float] = None) -> Union[Optional[str], bytearray]:
        """
        Fixes a Blend.buf file

        See :meth:`BlendFile.correct` for more info

        Parameters
        ----------
        blendFile: Union[:class:`str`, :class:`bytes`]
            The file path to the Blend.buf file to fix

        modType: :class:`ModType`
            The type of mod to fix from

        modToFix: :class:`str`
            The name of the mod to fix to

        fixedBlendFile: Optional[:class:`str`]
            The file path for the fixed Blend.buf file :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        version: Optional[float]
            The game version to fix to :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will fix to the latest game version :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Raises
        ------
        :class:`BlendFileNotRecognized`
            If the original Blend.buf file provided by the parameter ``blendFile`` cannot be read

        :class:`BadBlendData`
            If the bytes passed into this function do not correspond to the format defined for a Blend.buf file

        Returns
        -------
        Union[Optional[:class:`str`], :class:`bytearray`]
            If the argument ``fixedBlendFile`` is ``None``, then will return an array of bytes for the fixed Blend.buf file :raw-html:`<br />` :raw-html:`<br />`
            Otherwise will return the filename to the fixed RemapBlend.buf file if the provided Blend.buf file got corrected
        """

        blend = BlendFile(blendFile)
        vgRemap = modType.getVGRemap(modToFix, version = version)
        return blend.correct(vgRemap = vgRemap, fixedBlendFile = fixedBlendFile)
    
    def correctBlend(self, fixedRemapBlends: Set[str], skippedBlends: Dict[str, Exception], fixOnly: bool = False) -> List[Union[Set[str], Dict[str, Exception]]]:
        """
        Fixes all the Blend.buf files reference by the mod

        Requires all the .ini files in the mod to have ran their :meth:`IniFile.parse` function

        Parameters
        ----------
        fixedRemapBlends: Set[:class:`str`]
            All of the RemapBlend.buf files that have already been fixed.

        skippedBlends: Dict[:class:`str`, :class:`Exception`]
            All of the RemapBlend.buf files that have already been skipped due to some error when trying to fix them :raw-html:`<br />` :raw-html:`<br />`

            The keys are the absolute filepath to the RemapBlend.buf file that was attempted to be fixed and the values are the exception encountered

        fixOnly: :class:`bool`
            Whether to not correct some Blend.buf file if its corresponding RemapBlend.buf already exists :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns
        -------
        [Set[:class:`str`], Dict[:class:`str`, :class:`Exception`]]
            #. The absolute file paths of the RemapBlend.buf files that were fixed
            #. The exceptions encountered when trying to fix some RemapBlend.buf files :raw-html:`<br />` :raw-html:`<br />`

            The keys are absolute filepath to the RemapBlend.buf file and the values are the exception encountered
        """

        currentBlendsSkipped = {}
        currentBlendsFixed = set()

        for ini in self.inis:
            if (ini is None):
                continue

            for _, model in ini.remapBlendModels.items():
                modType = self._defaultType
                if (ini.type is not None):
                    modType = ini.type

                for partIndex, partFullPaths in model.fullPaths.items():
                    for modName, fixedFullPath in partFullPaths.items():
                        try:
                            origFullPath = model.origFullPaths[partIndex]
                        except KeyError:
                            self.print("log", f"Missing Original Blend file for the RemapBlend file at {fixedFullPath}")
                            if (fixedFullPath not in skippedBlends):
                                error = RemapMissingBlendFile(fixedFullPath)
                                currentBlendsSkipped[fixedFullPath] = error
                                skippedBlends[fixedFullPath] = error
                            break

                        # check if the blend was already encountered and did not need to be fixed
                        if (origFullPath in fixedRemapBlends or modType is None):
                            break
                        
                        # check if the blend file that did not need to be fixed already had encountered an error
                        if (origFullPath in skippedBlends):
                            self.print("log", f"Blend file has already previously encountered an error at {origFullPath}")
                            break
                        
                        # check if the blend file has been fixed
                        if (fixedFullPath in fixedRemapBlends):
                            self.print("log", f"Blend file has already been corrected at {fixedFullPath}")
                            continue

                        # check if the blend file already had encountered an error
                        if (fixedFullPath in skippedBlends):
                            self.print("log", f"Blend file has already previously encountered an error at {fixedFullPath}")
                            continue

                        # check if the fixed RemapBlend.buf file already exists and we only want to fix mods without removing their previous fixes
                        if (fixOnly and os.path.isfile(fixedFullPath)):
                            self.print("log", f"Blend file was previously fixed at {fixedFullPath}")
                            continue
                        
                        # fix the blend
                        correctedBlendPath = None
                        try:
                            correctedBlendPath = self.blendCorrection(origFullPath, modType, modName, fixedBlendFile = fixedFullPath, version = self.version)
                        except Exception as e:
                            currentBlendsSkipped[fixedFullPath] = e
                            skippedBlends[fixedFullPath] = e
                            self.print("handleException", e)
                        else:
                            pathToAdd = ""
                            if (correctedBlendPath is None):
                                self.print("log", f"Blend file does not need to be corrected at {origFullPath}")
                                pathToAdd = origFullPath
                            else:
                                self.print("log", f'Blend file correction done at {fixedFullPath}')
                                pathToAdd = fixedFullPath

                            currentBlendsFixed.add(pathToAdd)
                            fixedRemapBlends.add(pathToAdd)

        return [currentBlendsFixed, currentBlendsSkipped]


class RemapService():
    """
    The overall class for remapping modss

    Parameters
    ----------
    path: Optional[:class:`str`]
        The file location of where to run the fix. :raw-html:`<br />` :raw-html:`<br />`

        If this attribute is set to ``None``, then will run the fix from wherever this class is called :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    keepBackups: :class:`bool`
        Whether to keep backup versions of any .ini files that the script fixes :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    fixOnly: :class:`bool`
        Whether to only fix the mods without removing any previous changes this fix script may have made :raw-html:`<br />` :raw-html:`<br />`

        .. warning::
            if this is set to ``True`` and :attr:`undoOnly` is also set to ``True``, then the fix will not run and will throw a :class:`ConflictingOptions` exception

        :raw-html:`<br />`

        **Default**: ``False``

    undoOnly: :class:`bool`
        Whether to only undo the fixes previously made by the fix :raw-html:`<br />` :raw-html:`<br />`

        .. warning::
            if this is set to ``True`` and :attr:`fixOnly` is also set to ``True``, then the fix will not run and will throw a :class:`ConflictingOptions` exception

        :raw-html:`<br />`

        **Default**: ``True``

    readAllInis: :class:`bool`
        Whether to read all the .ini files that the fix encounters :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    types: Optional[:class:`str`]
        A string containing the names for all the types of mods to fix. Each type of mod is seperated using a comma (,)  :raw-html:`<br />` :raw-html:`<br />`

        If this argument is the empty string or this argument is ``None``, then will fix all the types of mods supported by this fix :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    defaultType: Optional[:class:`str`]
        The name for the type to use if a mod has an unidentified type :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then mods with unidentified types will be skipped :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    log: Optional[:class:`str`]
        The folder location to log the run of the fix into a seperate text file :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will not log the fix :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    verbose: :class:`bool`
        Whether to print the progress for fixing mods :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    handleExceptions: :class:`bool`
        When an exception is caught, whether to silently stop running the fix :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    version: Optional[:class:`float`]
        The game version we want the fix to be compatible with :raw-html:`<br />` :raw-html:`<br />`

        If This value is ``None``, then will retrieve the hashes/indices of the latest version. :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    _loggerBasePrefix: :class:`str`
        The prefix string for the logger used when the fix returns back to the original directory that it started to run

    logger: :class:`Logger`
        The logger used to pretty print messages

    _path: :class:`str`
        The file location of where to run the fix.

    keepBackups: :class:`bool`
        Whether to keep backup versions of any .ini files that the script fixes

    fixOnly: :class:`bool`
        Whether to only fix the mods without removing any previous changes this fix script may have made

    undoOnly: :class:`bool`
        Whether to only undo the fixes previously made by the fix

    readAllInis: :class:`bool`
        Whether to read all the .ini files that the fix encounters

    types: Set[:class:`ModType`]
        All the types of mods that will be fixed.

    defaultType: Optional[:class:`ModType`]
        The type to use if a mod has an unidentified type

    verbose: :class:`bool`
        Whether to print the progress for fixing mods

    version: Optional[:class:`float`]
        The game version we want the fix to be compatible with :raw-html:`<br />` :raw-html:`<br />`

        If This value is ``None``, then will retrieve the hashes/indices of the latest version.

    handleExceptions: :class:`bool`
        When an exception is caught, whether to silently stop running the fix

    _logFile: :class:`str`
        The file path of where to generate a log .txt file

    _pathIsCWD: :class:`bool`
        Whether the filepath that the program runs from is the current directory where this module is loaded

    modsFixed: :class:`int`
        The number of mods that have been fixed

    skippedMods: Dict[:class:`str`, :class:`Exception`]
        All the mods that have been skipped :raw-html:`<br />` :raw-html:`<br />`

        The keys are the absolute path to the mod folder and the values are the exception that caused the mod to be skipped

    blendsFixed: Set[:class:`str`]
        The absolute paths to all the Blend.buf files that have been fixed

    skippedBlendsByMods: DefaultDict[:class:`str`, Dict[:class:`str`, :class:`Exception`]]
        The RemapBlend.buf files that got skipped :raw-html for each mod :raw-html:`<br />` :raw-html:`<br />`

        * The outer key is the absolute path to the mod folder
        * The inner key is the absolute path to the RemapBlend.buf file
        * The value in the inner dictionary is the exception that caused the RemapBlend.buf file to be skipped

    skippedBlends: Dict[:class:`str`, :class:`Exception`]
        The RemapBlend.buf files that got skipped  :raw-html:`<br />` :raw-html:`<br />`

        The keys are the absolute path to the RemapBlend.buf file and the values are the exception that caused the RemapBlend.buf file to be skipped

    inisFixed: Set[:class:`str`]
        The absolute paths to the fixed .ini files

    inisSkipped: Dict[:class:`str`, :class:`Exception`]
        The .ini files that got skipped :raw-html:`<br />` :raw-html:`<br />`

        The keys are the absolute file paths to the .ini files and the values are exceptions that caused the .ini file to be skipped

    removedRemapBlends: Set[:class:`str`]
        Previous RemapBlend.buf files that are removed

    undoedInis: Set[:class:`str`]
        .ini files that got cleared out of any traces of previous fixes

        .. note::
            These .ini files may or may not have been previously fixed. A path to some .ini file in this attribute **DOES NOT** imply
            that the .ini file previously had a fix
    """

    def __init__(self, path: Optional[str] = None, keepBackups: bool = True, fixOnly: bool = False, undoOnly: bool = False, 
                 readAllInis: bool = False, types: Optional[str] = None, defaultType: Optional[str] = None, log: Optional[str] = None, 
                 verbose: bool = True, handleExceptions: bool = False, version: Optional[float] = None):
        self.log = log
        self._loggerBasePrefix = ""
        self.logger = Logger(logTxt = log, verbose = verbose)
        self._path = path
        self.keepBackups = keepBackups
        self.fixOnly = fixOnly
        self.undoOnly = undoOnly
        self.readAllInis = readAllInis
        self.types = types
        self.defaultType = defaultType
        self.verbose = verbose
        self.version = version
        self.handleExceptions = handleExceptions
        self._pathIsCwd = False
        self.__errorsBeforeFix = None

        # certain statistics about the fix
        self.modsFixed = 0
        self.skippedMods: Dict[str, Exception] = {}
        self.blendsFixed: Set[str] = set()
        self.skippedBlendsByMods: DefaultDict[str, Dict[str, Exception]] = defaultdict(lambda: {})
        self.skippedBlends: Dict[str, Exception] = {}
        self.inisFixed = set()
        self.inisSkipped: Dict[str, Exception] = {}
        self.removedRemapBlends: Set[str] = set()
        self.undoedInis: Set[str] = set()
        self._visitedRemapBlendsAtRemoval: Set[str] = set()

        self._setupModPath()
        self._setupModTypes()
        self._setupDefaultModType()

        if (self.__errorsBeforeFix is None):
            self._printModsToFix()

    @property
    def pathIsCwd(self):
        """
        Whether the filepath that the program runs from is the current directory where this module is loaded

        :getter: Returns whether the filepath that the program runs from is the current directory of where the module is loaded
        :type: :class:`bool`
        """

        return self._pathIsCwd
    
    @property
    def path(self) -> str:
        """
        The filepath of where the fix is running from

        :getter: Returns the path of where the fix is running
        :setter: Sets the path for where the fix runs
        :type: :class:`str`
        """

        return self._path
    
    @path.setter
    def path(self, newPath: Optional[str]):
        self._path = newPath
        self._setupModPath()
        self.clear()

    @property
    def log(self) -> str:
        """
        The folder location to log the run of the fix into a seperate text file

        :getter: Returns the file path to the log
        :setter: Sets the path for the log
        :type: :class:`str`
        """

        return self._log
    
    @log.setter
    def log(self, newLog: Optional[str]):
        self._log = newLog
        self._setupLogPath()

    def clear(self, clearLog: bool = True):
        """
        Clears up all the saved data

        Paramters
        ---------
        clearLog: :class:`bool`
            Whether to also clear out any saved data in the logger
        """

        self.modsFixed = 0
        self.skippedMods = {}
        self.blendsFixed = set()
        self.skippedBlendsByMods = defaultdict(lambda: {})
        self.skippedBlends = {}
        self.inisFixed = set()
        self.inisSkipped = {}
        self.removedRemapBlends = set()
        self.undoedInis = set()
        self._visitedRemapBlendsAtRemoval = set()

        if (clearLog):
            self.logger.clear()
    
    def _setupModPath(self):
        """
        Sets the filepath of where the fix will run from
        """

        self._pathIsCwd = False
        if (self._path is None):
            self._path = DefaultPath
            self._pathIsCwd = True
            return

        self._path = FileService.parseOSPath(self._path)
        self._path = FileService.parseOSPath(os.path.abspath(self._path))
        self._pathIsCwd = (self._path == DefaultPath)

    def _setupLogPath(self):
        """
        Sets the folder path for where the log file will be stored
        """

        if (self._log is not None):
            self._log = FileService.parseOSPath(os.path.join(self._log, LogFile))

    def _setupModTypes(self):
        """
        Sets the types of mods that will be fixed
        """

        if (isinstance(self.types, set)):
            return

        modTypes = set()
        if (self.types is None or self.readAllInis):
            modTypes = ModTypes.getAll()

        # search for the types of mods to fix
        else:
            typesLst = self.types.split(",")

            for typeStr in typesLst:
                modType = ModTypes.search(typeStr)
                modTypeFound = bool(modType is not None)

                if (modTypeFound):
                    modTypes.add(modType)
                elif (self.__errorsBeforeFix is None):
                    self.__errorsBeforeFix = InvalidModType(typeStr)
                    return

        self.types = modTypes

    def _setupDefaultModType(self):
        """
        Sets the default mod type to be used for an unidentified mod
        """

        if (not self.readAllInis):
            self.defaultType = None
        elif (self.defaultType is None):
            self.defaultType = ModTypes.Raiden.value
            return

        if (self.defaultType is None or isinstance(self.defaultType, ModType)):
            return

        self.defaultType = ModTypes.search(self.defaultType)

        if (self.defaultType is None and self.__errorsBeforeFix is None):
            self.__errorsBeforeFix = InvalidModType(self.defaultType)

    def _printModsToFix(self):
        """
        Prints out the types of mods that will be fixed
        """

        self.logger.includePrefix = False

        self.logger.openHeading("Types of Mods To Fix", 5)
        self.logger.space()

        if (not self.types):
            self.logger.log("All mods")
        else:
            for type in self.types:
                self.logger.bulletPoint(f"{type.name}")
        
        self.logger.space()
        self.logger.closeHeading()
        self.logger.split() 
        self.logger.includePrefix = True
    
    # fixes an ini file in a mod
    def fixIni(self, ini: IniFile, mod: Mod, fixedRemapBlends: Set[str]) -> bool:
        """
        Fixes an individual .ini file for a particular mod

        .. note:: 
            For more info about how we define a 'mod', go to :class:`Mod`

        Parameters
        ----------
        ini: :class:`IniFile`
            The .ini file to fix

        mod: :class:`Mod`
            The mod being fixed

        fixedRemapBlends: Set[:class:`str`]
            All of the RemapBlend.buf files that have already been fixed.

        Returns
        -------
        :class:`bool`
            Whether the particular .ini file has just been fixed
        """

        # check if the .ini is belongs to some mod
        if (ini is None or not ini.isModIni):
            return False

        if (self.undoOnly):
            return True

        fileBaseName = os.path.basename(ini.file)
        iniFullPath = FileService.absPathOfRelPath(ini.file, mod.path)

        if (iniFullPath in self.inisSkipped):
            self.logger.log(f"the ini file, {fileBaseName}, has alreaedy encountered an error")
            return False
        
        if (iniFullPath in self.inisFixed):
            self.logger.log(f"the ini file, {fileBaseName}, is already fixed")
            return True

        # parse the .ini file
        self.logger.log(f"Parsing {fileBaseName}...")
        ini.parse()

        if (ini.isFixed):
            self.logger.log(f"the ini file, {fileBaseName}, is already fixed")
            return True

        # fix the blends
        self.logger.log(f"Fixing the {FileTypes.Blend.value} files for {fileBaseName}...")
        currentBlendsFixed, currentBlendsSkipped = mod.correctBlend(fixedRemapBlends = fixedRemapBlends, skippedBlends = self.skippedBlends, fixOnly = self.fixOnly)
        self.blendsFixed = self.blendsFixed.union(currentBlendsFixed)

        if (currentBlendsSkipped):
            DictTools.update(self.skippedBlendsByMods[mod.path], currentBlendsSkipped)

        # writing the fixed file
        self.logger.log(f"Making the fixed ini file for {fileBaseName}")
        ini.fix(keepBackup = self.keepBackups, fixOnly = self.fixOnly)

        return True

    # fixes a mod
    def fixMod(self, mod: Mod, fixedRemapBlends: Set[str]) -> bool:
        """
        Fixes a particular mod

        .. note:: 
            For more info about how we define a 'mod', go to :class:`Mod`

        Parameters
        ----------
        mod: :class:`Mod`
            The mod being fixed

        fixedRemapBlends: Set[:class:`str`]
            all of the RemapBlend.buf files that have already been fixed.

        Returns
        -------
        :class:`bool`
            Whether the particular mod has just been fixed
        """

        # remove any backups
        if (not self.keepBackups):
            mod.removeBackupInis()

        for ini in mod.inis:
            ini.checkIsMod()

        # undo any previous fixes
        if (not self.fixOnly):
            undoedInis, removedRemapBlends = mod.removeFix(self.blendsFixed, self.inisFixed, self._visitedRemapBlendsAtRemoval, self.inisSkipped, keepBackups = self.keepBackups, fixOnly = self.fixOnly)
            self.removedRemapBlends = self.removedRemapBlends.union(removedRemapBlends)
            self.undoedInis = self.undoedInis.union(undoedInis)

        result = False
        firstIniException = None
        inisLen = len(mod.inis)

        for i in range(inisLen):
            ini = mod.inis[i]
            iniFullPath = FileService.absPathOfRelPath(ini.file, mod.path)
            iniIsFixed = False

            try:
                iniIsFixed = self.fixIni(ini, mod, fixedRemapBlends)
            except Exception as e:
                self.logger.handleException(e)
                self.inisSkipped[iniFullPath] = e 

                if (firstIniException is None):
                    firstIniException = e

            if (firstIniException is None and iniFullPath in self.inisSkipped):
                firstIniException = self.inisSkipped[iniFullPath]

            result = (result or iniIsFixed)

            if (not iniIsFixed):
                continue
            
            if (i < inisLen - 1):
                self.logger.space()

            self.inisFixed.add(iniFullPath)

        if (not result and firstIniException is not None):
            self.skippedMods[mod.path] = firstIniException

        return result
    
    def addTips(self):
        """
        Prints out any useful tips for the user to know
        """

        self.logger.includePrefix = False

        if (not self.undoOnly or self.keepBackups):
            self.logger.split()
            self.logger.openHeading("Tips", sideLen = 10)

            if (self.keepBackups):
                self.logger.bulletPoint(f'Hate deleting the "{BackupFilePrefix}" {FileExt.Ini.value}/{FileExt.Txt.value} files yourself after running this script? (cuz I know I do!) Run this script again (on CMD) using the {DeleteBackupOpt} option')

            if (not self.undoOnly):
                self.logger.bulletPoint(f"Want to undo this script's fix? Run this script again (on CMD) using the {RevertOpt} option")

            if (not self.readAllInis):
                self.logger.bulletPoint(f"Were your {FileTypes.Ini.value}s not read? Run this script again (on CMD) using the {AllOpt} option")

            self.logger.space()
            self.logger.log("For more info on command options, run this script (on CMD) using the --help option")
            self.logger.closeHeading()

        self.logger.includePrefix = True


    def reportSkippedAsset(self, assetName: str, assetDict: Dict[str, Exception], warnStrFunc: Callable[[str], str]):
        """
        Prints out the exception message for why a particular .ini file or Blend.buf file has been skipped

        Parameters
        ----------
        assetName: :class:`str`
            The name for the type of asset (files, folders, mods, etc...) that was skipped

        assetDict: Dict[:class:`str`, :class:`Exception`]
            Locations of where exceptions have occured for the particular asset :raw-html:`<br />` :raw-html:`<br />`

            The keys are the absolute folder paths to where the exception occured

        wantStrFunc: Callable[[:class:`str`], :class:`str`]
            Function for how we want to print out the warning for each exception :raw-html:`<br />` :raw-html:`<br />`

            Takes in the folder location of where the exception occured as a parameter
        """

        if (assetDict):
            message = f"\nWARNING: The following {assetName} were skipped due to warnings (see log above):\n\n"
            for dir in assetDict:
                message += warnStrFunc(dir)

            self.logger.error(message)
            self.logger.space()

    def warnSkippedBlends(self, modPath: str):
        """
        Prints out all of the Blend.buf files that were skipped due to exceptions

        Parameters
        ----------
        modPath: :class:`str`
            The absolute path to a particular folder
        """

        parentFolder = os.path.dirname(self._path)
        relModPath = FileService.getRelPath(modPath, parentFolder)
        modHeading = Heading(f"Mod: {relModPath}", 5)
        message = f"{modHeading.open()}\n\n"
        blendWarnings = self.skippedBlendsByMods[modPath]
        
        for blendPath in blendWarnings:
            relBlendPath = FileService.getRelPath(blendPath, self._path)
            message += self.logger.getBulletStr(f"{relBlendPath}:\n\t{Heading(type(blendWarnings[blendPath]).__name__, 3, '-').open()}\n\t{blendWarnings[blendPath]}\n\n")
        
        message += f"{modHeading.close()}\n"
        return message

    def reportSkippedMods(self):
        """
        Prints out all of the mods that were skipped due to exceptions

        .. note:: 
            For more info about how we define a 'mod', go to :class:`Mod`
        """

        self.reportSkippedAsset("mods", self.skippedMods, lambda dir: self.logger.getBulletStr(f"{dir}:\n\t{Heading(type(self.skippedMods[dir]).__name__, 3, '-').open()}\n\t{self.skippedMods[dir]}\n\n"))
        self.reportSkippedAsset(f"{FileTypes.Ini.value}s", self.inisSkipped, lambda file: self.logger.getBulletStr(f"{file}:\n\t{Heading(type(self.inisSkipped[file]).__name__, 3, '-').open()}\n\t{self.inisSkipped[file]}\n\n"))
        self.reportSkippedAsset(f"{FileTypes.Blend.value} files", self.skippedBlendsByMods, lambda dir: self.warnSkippedBlends(dir))

    def reportSummary(self):
        skippedMods = len(self.skippedMods)
        foundMods = self.modsFixed + skippedMods
        fixedBlends = len(self.blendsFixed)
        skippedBlends = len(self.skippedBlends)
        foundBlends = fixedBlends + skippedBlends
        fixedInis = len(self.inisFixed)
        skippedInis = len(self.inisSkipped)
        foundInis = fixedInis + skippedInis
        removedRemapBlends = len(self.removedRemapBlends)
        undoedInis = len(self.undoedInis)

        self.logger.openHeading("Summary", sideLen = 10)
        self.logger.space()
        
        modFixMsg = ""
        blendFixMsg = ""
        iniFixMsg = ""
        removedRemappedMsg = ""
        undoedInisMsg = ""
        if (not self.undoOnly):
            modFixMsg = f"Out of {foundMods} found mods, fixed {self.modsFixed} mods and skipped {skippedMods} mods"
            iniFixMsg = f"Out of the {foundInis} {FileTypes.Ini.value}s within the found mods, fixed {fixedInis} {FileTypes.Ini.value}s and skipped {skippedInis} {FileTypes.Ini.value}s"
            blendFixMsg = f"Out of the {foundBlends} {FileTypes.Blend.value} files within the found mods, fixed {fixedBlends} {FileTypes.Blend.value} files and skipped {skippedBlends} {FileTypes.Blend.value} files"
        else:
            modFixMsg = f"Out of {foundMods} found mods, remove fix from {self.modsFixed} mods and skipped {skippedMods} mods"

        if (not self.fixOnly and undoedInis > 0):
            undoedInisMsg = f"Removed fix from up to {undoedInis} {FileTypes.Ini.value}s"

            if (self.undoOnly):
                undoedInisMsg += f" and skipped {skippedInis} {FileTypes.Ini.value}s"

        if (not self.fixOnly and removedRemapBlends > 0):
            removedRemappedMsg = f"Removed {removedRemapBlends} old {RemapBlendFile} files"


        self.logger.bulletPoint(modFixMsg)
        if (iniFixMsg):
            self.logger.bulletPoint(iniFixMsg)

        if (blendFixMsg):
            self.logger.bulletPoint(blendFixMsg)

        if (undoedInisMsg):
            self.logger.bulletPoint(undoedInisMsg)

        if (removedRemappedMsg):
            self.logger.bulletPoint(removedRemappedMsg)

        self.logger.space()
        self.logger.closeHeading()

    def createLog(self):
        """
        Creates a log text file that contains all the text printed on the command line
        """

        if (self._log is None):
            return

        self.logger.includePrefix = False
        self.logger.space()

        self.logger.log(f"Creating log file, {LogFile}")

        self.logger.includePrefix = True

        with open(self._log, "w", encoding = IniFileEncoding) as f:
            f.write(self.logger.loggedTxt)

    def createMod(self, path: Optional[str] = None, files: Optional[List[str]] = None) -> Mod:
        """
        Creates a mod

        .. note:: 
            For more info about how we define a 'mod', go to :class:`Mod`

        Parameters
        ----------
        path: Optional[:class:`str`]
            The absolute path to the mod folder. :raw-html:`<br />` :raw-html:`<br />`
            
            If this argument is set to ``None``, then will use the current directory of where this module is loaded

        files: Optional[List[:class:`str`]]
            The direct children files to the mod folder (does not include files located in a folder within the mod folder). :raw-html:`<br />` :raw-html:`<br />`

            If this parameter is set to ``None``, then the module will search the folders for you

        Returns
        -------
        :class:`Mod`
            The mod that has been created
        """

        path = FileService.getPath(path)
        mod = Mod(path = path, files = files, logger = self.logger, types = self.types, defaultType = self.defaultType, version = self.version)
        return mod

    def _fix(self):
        """
        The overall logic for fixing a bunch of mods

        For finding out which folders may contain mods, this function:
            #. recursively searches all folders from where the :attr:`RemapService.path` is located
            #. for every .ini file in a valid mod and every Blend.buf file encountered that is encountered, recursively search all the folders from where the .ini file or Blend.buf file is located

        .. note:: 
            For more info about how we define a 'mod', go to :class:`Mod`
        """

        if (self.__errorsBeforeFix is not None):
            raise self.__errorsBeforeFix

        if (self.fixOnly and self.undoOnly):
            raise ConflictingOptions([FixOnlyOpt, RevertOpt])

        parentFolder = os.path.dirname(self._path)
        self._loggerBasePrefix = os.path.basename(self._path)
        self.logger.prefix = os.path.basename(DefaultPath)

        visitedDirs = set()
        visitingDirs = set()
        dirs = deque()
        dirs.append(self._path)
        visitingDirs.add(self._path)
        fixedRemapBlends = set()
    
        while (dirs):
            path = dirs.popleft()
            fixedMod = False

            # skip if the directory has already been visited
            if (path in visitedDirs):
                visitingDirs.remove(path)
                visitedDirs.add(path)
                continue 
            
            self.logger.split()

            # get the relative path to where the program runs
            self.logger.prefix = FileService.getRelPath(path, parentFolder)

            # try to make the mod, skip if cannot be made
            try:
                mod = self.createMod(path = path)
            except Exception as e:
                visitingDirs.remove(path)
                visitedDirs.add(path)
                continue
            
            # fix the mod
            try:
                fixedMod = self.fixMod(mod, fixedRemapBlends)
            except Exception as e:
                self.logger.handleException(e)
                if (mod.inis):
                    self.skippedMods[path] = e

            # get all the folders that could potentially be other mods
            modFiles, modDirs = FileService.getFilesAndDirs(path = path, recursive = True)

            if (mod.inis):
                for ini in mod.inis:
                    for _, blendModel in ini.remapBlendModels.items():
                        resourceModDirs = map(lambda partIndex: os.path.dirname(blendModel.origFullPaths[partIndex]), blendModel.origFullPaths) 
                        modDirs += resourceModDirs
            
            # add in all the folders that need to be visited
            for dir in modDirs:
                if (dir in visitedDirs):
                    continue

                if (dir not in visitingDirs):
                    dirs.append(dir)
                visitingDirs.add(dir)

            # increment the count of mods found
            if (fixedMod):
                self.modsFixed += 1

            visitingDirs.remove(path)
            visitedDirs.add(path)

        self.logger.split()
        self.logger.prefix = self._loggerBasePrefix
        self.reportSkippedMods()
        self.logger.space()
        self.reportSummary()


    def fix(self):
        """
        Fixes a bunch of mods

        see :meth:`_fix` for more info
        """
        
        try:
            self._fix()
        except Exception as e:
            if (self.handleExceptions):
                self.logger.handleException(e)
            else:
                self.createLog()
                raise e from e
        else:
            noErrors = bool(not self.skippedMods and not self.skippedBlendsByMods)

            if (noErrors):
                self.logger.space()
                self.logger.log("ENJOY")

            self.logger.split()

            if (noErrors):
                self.addTips()

        self.createLog()


def main():
    command = CommandBuilder()
    command.addEpilog(ModTypes.getHelpStr())

    args = command.parseArgs()
    readAllInis = args.all
    defaultType = args.defaultType

    remapService = RemapService(path = args.src, keepBackups = not args.deleteBackup, fixOnly = args.fixOnly, 
                                    undoOnly = args.revert, readAllInis = readAllInis, types = args.types, defaultType = defaultType,
                                    log = args.log, verbose = True, handleExceptions = True)
    remapService.fix()
    remapService.logger.waitExit()

# Main Driver Code
if __name__ == "__main__":
    main()