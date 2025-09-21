import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class IfPredParserTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._tokenizer = FRB.IfPredTokenizer()
        cls._parser = FRB.IfPredParser(setup = False)

        cls._prodId = 0
        cls._stateId = 0

        cls._tokenizer.setup()
        #cls._parser.setup()

    def setUp(self):
        super().setUp()

        self._prodId = 0
        self._stateId = 0
        self._nodeId = 0

        self.patch("src.FixRaidenBoss2.BaseSLR1Parser._generateStateId", side_effect = self._generateStateId)
        self.patch("src.FixRaidenBoss2.BaseSLR1Parser._generateProductionId", side_effect = self._generateProductionId)
        self.patch("src.FixRaidenBoss2.BaseSLR1Parser._generateParserNodeId", side_effect = self._generateNodeId)

        self._parser.setup()

    def _generateStateId(self) -> int:
        self._prodId += 1
        return self._prodId
    
    def _generateProductionId(self) -> int:
        self._stateId -= 1
        return self._stateId
    
    def _generateNodeId(self) -> int:
        self._nodeId += 1
        return self._nodeId

    # ================== parse =======================

    def test_differentIfPreds_ifPredsParsed(self):
        file = "MyFile.agr"
        startLineNo = 8

        tests = [
                    [" ", 
                     FRB.ParseTree({
                         1: FRB.ParseNode(1, token=FRB.Token('STARTTOKEN', 'STARTTOKEN', 8, 0)),
                         2: FRB.ParseNode(2, token=FRB.Token('ENDTOKEN', 'ENDTOKEN', 9, 0)),
                         3: FRB.ParseNode(3, prodId="start empty"),
                     },
                     {3: [1, 2]},
                     3)],

                    ["$swapvar == 5", 
                     FRB.ParseTree({
                        1: FRB.ParseNode(1, token = FRB.Token('STARTTOKEN', 'STARTTOKEN', 8, 0)),
                        2: FRB.ParseNode(2, token = FRB.Token('ID', '$swapvar', 8, 1)),
                        3: FRB.ParseNode(3, prodId = 'variable'),
                        4: FRB.ParseNode(4, prodId = 'multexpr reduce'),
                        5: FRB.ParseNode(5, prodId = 'addexpr reduce'),
                        6: FRB.ParseNode(6, prodId = 'keyexpr reduce'),
                        7: FRB.ParseNode(7, token = FRB.Token('EQ', '==', 8, 10)),
                        8: FRB.ParseNode(8, token = FRB.Token('INT', '5', 8, 13)),
                        9: FRB.ParseNode(9, prodId = 'int'),
                        10: FRB.ParseNode(10, prodId = 'multexpr reduce'),
                        11: FRB.ParseNode(11, prodId = 'addexpr reduce'),
                        12: FRB.ParseNode(12, prodId = 'keyexpr reduce'),
                        13: FRB.ParseNode(13, prodId = 'eq'),
                        14: FRB.ParseNode(14, prodId = 'ntest reduce'),
                        15: FRB.ParseNode(15, prodId = 'pred reduce'),
                        16: FRB.ParseNode(16, token = FRB.Token('ENDTOKEN', 'ENDTOKEN', 9, 0)),
                        17: FRB.ParseNode(17, prodId = 'start')
                    },
                    {3: [2], 4: [3], 5: [4], 6: [5], 9: [8], 10: [9], 11: [10], 12: [11], 13: [6, 7, 12], 14: [13], 15: [14], 17: [1, 15, 16]},
                    17)],

                    ['$x >= 5', 
                     FRB.ParseTree({
                        1: FRB.ParseNode(1, token = FRB.Token('STARTTOKEN', 'STARTTOKEN', 8, 0)),
                        2: FRB.ParseNode(2, token = FRB.Token('ID', '$x', 8, 1)),
                        3: FRB.ParseNode(3, prodId = 'variable'),
                        4: FRB.ParseNode(4, prodId = 'multexpr reduce'),
                        5: FRB.ParseNode(5, prodId = 'addexpr reduce'),
                        6: FRB.ParseNode(6, prodId = 'keyexpr reduce'),
                        7: FRB.ParseNode(7, token = FRB.Token('GE', '>=', 8, 4)),
                        8: FRB.ParseNode(8, token = FRB.Token('INT', '5', 8, 7)),
                        9: FRB.ParseNode(9, prodId = 'int'),
                        10: FRB.ParseNode(10, prodId = 'multexpr reduce'),
                        11: FRB.ParseNode(11, prodId = 'addexpr reduce'),
                        12: FRB.ParseNode(12, prodId = 'keyexpr reduce'),
                        13: FRB.ParseNode(13, prodId = 'ge'),
                        14: FRB.ParseNode(14, prodId = 'ntest reduce'),
                        15: FRB.ParseNode(15, prodId = 'pred reduce'),
                        16: FRB.ParseNode(16, token = FRB.Token('ENDTOKEN', 'ENDTOKEN', 9, 0)),
                        17: FRB.ParseNode(17, prodId = 'start')
                     }, {3: [2], 4: [3], 5: [4], 6: [5], 9: [8], 10: [9], 11: [10], 12: [11], 13: [6, 7, 12], 14: [13], 15: [14], 17: [1, 15, 16]},
                     17)],

                     ['($/swapvar < 7)', 
                      FRB.ParseTree({
                        1: FRB.ParseNode(1, token = FRB.Token('STARTTOKEN', 'STARTTOKEN', 8, 0)),
                        2: FRB.ParseNode(2, token = FRB.Token('LPAREN', '(', 8, 1)),
                        3: FRB.ParseNode(3, token = FRB.Token('ID', '$/swapvar', 8, 2)),
                        4: FRB.ParseNode(4, prodId = 'variable'),
                        5: FRB.ParseNode(5, prodId = 'multexpr reduce'),
                        6: FRB.ParseNode(6, prodId = 'addexpr reduce'),
                        7: FRB.ParseNode(7, prodId = 'keyexpr reduce'),
                        8: FRB.ParseNode(8, token = FRB.Token('LT', '<', 8, 12)),
                        9: FRB.ParseNode(9, token = FRB.Token('INT', '7', 8, 14)),
                        10: FRB.ParseNode(10, prodId = 'int'),
                        11: FRB.ParseNode(11, prodId = 'multexpr reduce'),
                        12: FRB.ParseNode(12, prodId = 'addexpr reduce'),
                        13: FRB.ParseNode(13, prodId = 'keyexpr reduce'),
                        14: FRB.ParseNode(14, prodId = 'lt'),
                        15: FRB.ParseNode(15, prodId = 'ntest reduce'),
                        16: FRB.ParseNode(16, prodId = 'pred reduce'),
                        17: FRB.ParseNode(17, token = FRB.Token('RPAREN', ')', 8, 15)),
                        18: FRB.ParseNode(18, prodId = 'bracket loop'),
                        19: FRB.ParseNode(19, prodId = 'multexpr reduce'),
                        20: FRB.ParseNode(20, prodId = 'addexpr reduce'),
                        21: FRB.ParseNode(21, prodId = 'keyexpr reduce'),
                        22: FRB.ParseNode(22, prodId = 'test reduce'),
                        23: FRB.ParseNode(23, prodId = 'ntest reduce'),
                        24: FRB.ParseNode(24, prodId = 'pred reduce'),
                        25: FRB.ParseNode(25, token = FRB.Token('ENDTOKEN', 'ENDTOKEN', 9, 0)),
                        26: FRB.ParseNode(26, prodId = 'start')
                      }, {4: [3], 5: [4], 6: [5], 7: [6], 10: [9], 11: [10], 12: [11], 13: [12], 14: [7, 8, 13], 15: [14], 16: [15], 18: [2, 16, 17], 19: [18], 20: [19], 21: [20], 22: [21], 23: [22], 24: [23], 26: [1, 24, 25]},
                      26)],

                      ['(($x >= 5 && $/swapvar < (7 + 3.456 * -0.0)) || !($z[]%d >= 1 && $z[]%d < -9 && $123 == null)) \t && $*! + -234 -        -0 / -9.6',
                       FRB.ParseTree({
                            1: FRB.ParseNode(1, token = FRB.Token('STARTTOKEN', 'STARTTOKEN', 8, 0)),
                            2: FRB.ParseNode(2, token = FRB.Token('LPAREN', '(', 8, 1)),
                            3: FRB.ParseNode(3, token = FRB.Token('LPAREN', '(', 8, 2)),
                            4: FRB.ParseNode(4, token = FRB.Token('ID', '$x', 8, 3)),
                            5: FRB.ParseNode(5, prodId = 'variable'),
                            6: FRB.ParseNode(6, prodId = 'multexpr reduce'),
                            7: FRB.ParseNode(7, prodId = 'addexpr reduce'),
                            8: FRB.ParseNode(8, prodId = 'keyexpr reduce'),
                            9: FRB.ParseNode(9, token = FRB.Token('GE', '>=', 8, 6)),
                            10: FRB.ParseNode(10, token = FRB.Token('INT', '5', 8, 9)),
                            11: FRB.ParseNode(11, prodId = 'int'),
                            12: FRB.ParseNode(12, prodId = 'multexpr reduce'),
                            13: FRB.ParseNode(13, prodId = 'addexpr reduce'),
                            14: FRB.ParseNode(14, prodId = 'keyexpr reduce'),
                            15: FRB.ParseNode(15, prodId = 'ge'),
                            16: FRB.ParseNode(16, prodId = 'ntest reduce'),
                            17: FRB.ParseNode(17, prodId = 'pred reduce'),
                            18: FRB.ParseNode(18, token = FRB.Token('AND', '&&', 8, 11)),
                            19: FRB.ParseNode(19, token = FRB.Token('ID', '$/swapvar', 8, 14)),
                            20: FRB.ParseNode(20, prodId = 'variable'),
                            21: FRB.ParseNode(21, prodId = 'multexpr reduce'),
                            22: FRB.ParseNode(22, prodId = 'addexpr reduce'),
                            23: FRB.ParseNode(23, prodId = 'keyexpr reduce'),
                            24: FRB.ParseNode(24, token = FRB.Token('LT', '<', 8, 24)),
                            25: FRB.ParseNode(25, token = FRB.Token('LPAREN', '(', 8, 26)),
                            26: FRB.ParseNode(26, token = FRB.Token('INT', '7', 8, 27)),
                            27: FRB.ParseNode(27, prodId = 'int'),
                            28: FRB.ParseNode(28, prodId = 'multexpr reduce'),
                            29: FRB.ParseNode(29, prodId = 'addexpr reduce'),
                            30: FRB.ParseNode(30, token = FRB.Token('PLUS', '+', 8, 29)),
                            31: FRB.ParseNode(31, token = FRB.Token('FLOAT', '3.456', 8, 31)),
                            32: FRB.ParseNode(32, prodId = 'float'),
                            33: FRB.ParseNode(33, prodId = 'multexpr reduce'),
                            34: FRB.ParseNode(34, token = FRB.Token('STAR', '*', 8, 37)),
                            35: FRB.ParseNode(35, token = FRB.Token('FLOAT', '-0.0', 8, 39)),
                            36: FRB.ParseNode(36, prodId = 'float'),
                            37: FRB.ParseNode(37, prodId = 'multiply'),
                            38: FRB.ParseNode(38, prodId = 'add'),
                            39: FRB.ParseNode(39, prodId = 'keyexpr reduce'),
                            40: FRB.ParseNode(40, prodId = 'test reduce'),
                            41: FRB.ParseNode(41, prodId = 'ntest reduce'),
                            42: FRB.ParseNode(42, prodId = 'pred reduce'),
                            43: FRB.ParseNode(43, token = FRB.Token('RPAREN', ')', 8, 43)),
                            44: FRB.ParseNode(44, prodId = 'bracket loop'),
                            45: FRB.ParseNode(45, prodId = 'multexpr reduce'),
                            46: FRB.ParseNode(46, prodId = 'addexpr reduce'),
                            47: FRB.ParseNode(47, prodId = 'keyexpr reduce'),
                            48: FRB.ParseNode(48, prodId = 'lt'),
                            49: FRB.ParseNode(49, prodId = 'ntest reduce'),
                            50: FRB.ParseNode(50, prodId = 'and'),
                            51: FRB.ParseNode(51, token = FRB.Token('RPAREN', ')', 8, 44)),
                            52: FRB.ParseNode(52, prodId = 'bracket loop'),
                            53: FRB.ParseNode(53, prodId = 'multexpr reduce'),
                            54: FRB.ParseNode(54, prodId = 'addexpr reduce'),
                            55: FRB.ParseNode(55, prodId = 'keyexpr reduce'),
                            56: FRB.ParseNode(56, prodId = 'test reduce'),
                            57: FRB.ParseNode(57, prodId = 'ntest reduce'),
                            58: FRB.ParseNode(58, prodId = 'pred reduce'),
                            59: FRB.ParseNode(59, token = FRB.Token('OR', '||', 8, 46)),
                            60: FRB.ParseNode(60, token = FRB.Token('NOT', '!', 8, 49)),
                            61: FRB.ParseNode(61, token = FRB.Token('LPAREN', '(', 8, 50)),
                            62: FRB.ParseNode(62, token = FRB.Token('ID', '$z[]%d', 8, 51)),
                            63: FRB.ParseNode(63, prodId = 'variable'),
                            64: FRB.ParseNode(64, prodId = 'multexpr reduce'),
                            65: FRB.ParseNode(65, prodId = 'addexpr reduce'),
                            66: FRB.ParseNode(66, prodId = 'keyexpr reduce'),
                            67: FRB.ParseNode(67, token = FRB.Token('GE', '>=', 8, 58)),
                            68: FRB.ParseNode(68, token = FRB.Token('INT', '1', 8, 61)),
                            69: FRB.ParseNode(69, prodId = 'int'),
                            70: FRB.ParseNode(70, prodId = 'multexpr reduce'),
                            71: FRB.ParseNode(71, prodId = 'addexpr reduce'),
                            72: FRB.ParseNode(72, prodId = 'keyexpr reduce'),
                            73: FRB.ParseNode(73, prodId = 'ge'),
                            74: FRB.ParseNode(74, prodId = 'ntest reduce'),
                            75: FRB.ParseNode(75, prodId = 'pred reduce'),
                            76: FRB.ParseNode(76, token = FRB.Token('AND', '&&', 8, 63)),
                            77: FRB.ParseNode(77, token = FRB.Token('ID', '$z[]%d', 8, 66)),
                            78: FRB.ParseNode(78, prodId = 'variable'),
                            79: FRB.ParseNode(79, prodId = 'multexpr reduce'),
                            80: FRB.ParseNode(80, prodId = 'addexpr reduce'),
                            81: FRB.ParseNode(81, prodId = 'keyexpr reduce'),
                            82: FRB.ParseNode(82, token = FRB.Token('LT', '<', 8, 73)),
                            83: FRB.ParseNode(83, token = FRB.Token('INT', '-9', 8, 75)),
                            84: FRB.ParseNode(84, prodId = 'int'),
                            85: FRB.ParseNode(85, prodId = 'multexpr reduce'),
                            86: FRB.ParseNode(86, prodId = 'addexpr reduce'),
                            87: FRB.ParseNode(87, prodId = 'keyexpr reduce'),
                            88: FRB.ParseNode(88, prodId = 'lt'),
                            89: FRB.ParseNode(89, prodId = 'ntest reduce'),
                            90: FRB.ParseNode(90, prodId = 'and'),
                            91: FRB.ParseNode(91, token = FRB.Token('AND', '&&', 8, 78)),
                            92: FRB.ParseNode(92, token = FRB.Token('ID', '$123', 8, 81)),
                            93: FRB.ParseNode(93, prodId = 'variable'),
                            94: FRB.ParseNode(94, prodId = 'multexpr reduce'),
                            95: FRB.ParseNode(95, prodId = 'addexpr reduce'),
                            96: FRB.ParseNode(96, prodId = 'keyexpr reduce'),
                            97: FRB.ParseNode(97, token = FRB.Token('EQ', '==', 8, 86)),
                            98: FRB.ParseNode(98, token = FRB.Token('NULL', 'null', 8, 89)),
                            99: FRB.ParseNode(99, prodId = 'null'),
                            100: FRB.ParseNode(100, prodId = 'eq'),
                            101: FRB.ParseNode(101, prodId = 'ntest reduce'),
                            102: FRB.ParseNode(102, prodId = 'and'),
                            103: FRB.ParseNode(103, token = FRB.Token('RPAREN', ')', 8, 93)),
                            104: FRB.ParseNode(104, prodId = 'bracket loop'),
                            105: FRB.ParseNode(105, prodId = 'multexpr reduce'),
                            106: FRB.ParseNode(106, prodId = 'addexpr reduce'),
                            107: FRB.ParseNode(107, prodId = 'keyexpr reduce'),
                            108: FRB.ParseNode(108, prodId = 'test reduce'),
                            109: FRB.ParseNode(109, prodId = 'not'),
                            110: FRB.ParseNode(110, prodId = 'or'),
                            111: FRB.ParseNode(111, token = FRB.Token('RPAREN', ')', 8, 94)),
                            112: FRB.ParseNode(112, prodId = 'bracket loop'),
                            113: FRB.ParseNode(113, prodId = 'multexpr reduce'),
                            114: FRB.ParseNode(114, prodId = 'addexpr reduce'),
                            115: FRB.ParseNode(115, prodId = 'keyexpr reduce'),
                            116: FRB.ParseNode(116, prodId = 'test reduce'),
                            117: FRB.ParseNode(117, prodId = 'ntest reduce'),
                            118: FRB.ParseNode(118, prodId = 'pred reduce'),
                            119: FRB.ParseNode(119, token = FRB.Token('AND', '&&', 8, 98)),
                            120: FRB.ParseNode(120, token = FRB.Token('ID', '$*!', 8, 101)),
                            121: FRB.ParseNode(121, prodId = 'variable'),
                            122: FRB.ParseNode(122, prodId = 'multexpr reduce'),
                            123: FRB.ParseNode(123, prodId = 'addexpr reduce'),
                            124: FRB.ParseNode(124, token = FRB.Token('PLUS', '+', 8, 105)),
                            125: FRB.ParseNode(125, token = FRB.Token('INT', '-234', 8, 107)),
                            126: FRB.ParseNode(126, prodId = 'int'),
                            127: FRB.ParseNode(127, prodId = 'multexpr reduce'),
                            128: FRB.ParseNode(128, prodId = 'add'),
                            129: FRB.ParseNode(129, token = FRB.Token('MINUS', '-', 8, 112)),
                            130: FRB.ParseNode(130, token = FRB.Token('INT', '-0', 8, 121)),
                            131: FRB.ParseNode(131, prodId = 'int'),
                            132: FRB.ParseNode(132, prodId = 'multexpr reduce'),
                            133: FRB.ParseNode(133, token = FRB.Token('SLASH', '/', 8, 124)),
                            134: FRB.ParseNode(134, token = FRB.Token('FLOAT', '-9.6', 8, 126)),
                            135: FRB.ParseNode(135, prodId = 'float'),
                            136: FRB.ParseNode(136, prodId = 'divide'),
                            137: FRB.ParseNode(137, prodId = 'subtract'),
                            138: FRB.ParseNode(138, prodId = 'keyexpr reduce'),
                            139: FRB.ParseNode(139, prodId = 'test reduce'),
                            140: FRB.ParseNode(140, prodId = 'ntest reduce'),
                            141: FRB.ParseNode(141, prodId = 'and'),
                            142: FRB.ParseNode(142, token = FRB.Token('ENDTOKEN', 'ENDTOKEN', 9, 0)),
                            143: FRB.ParseNode(143, prodId = 'start')
                       }, {5: [4], 6: [5], 7: [6], 8: [7], 11: [10], 12: [11], 13: [12], 14: [13], 15: [8, 9, 14], 16: [15], 17: [16], 20: [19], 21: [20], 22: [21], 23: [22], 
                           27: [26], 28: [27], 29: [28], 32: [31], 33: [32], 36: [35], 37: [33, 34, 36], 38: [29, 30, 37], 39: [38], 40: [39], 41: [40], 42: [41], 44: [25, 42, 43], 
                           45: [44], 46: [45], 47: [46], 48: [23, 24, 47], 49: [48], 50: [17, 18, 49], 52: [3, 50, 51], 53: [52], 54: [53], 55: [54], 56: [55], 57: [56], 58: [57], 
                           63: [62], 64: [63], 65: [64], 66: [65], 69: [68], 70: [69], 71: [70], 72: [71], 73: [66, 67, 72], 74: [73], 75: [74], 78: [77], 79: [78], 80: [79], 
                           81: [80], 84: [83], 85: [84], 86: [85], 87: [86], 88: [81, 82, 87], 89: [88], 90: [75, 76, 89], 93: [92], 94: [93], 95: [94], 96: [95], 99: [98], 
                           100: [96, 97, 99], 101: [100], 102: [90, 91, 101], 104: [61, 102, 103], 105: [104], 106: [105], 107: [106], 108: [107], 109: [60, 108], 110: [58, 59, 109], 
                           112: [2, 110, 111], 113: [112], 114: [113], 115: [114], 116: [115], 117: [116], 118: [117], 121: [120], 122: [121], 123: [122], 126: [125], 127: [126], 
                           128: [123, 124, 127], 131: [130], 132: [131], 135: [134], 136: [132, 133, 135], 137: [128, 129, 136], 138: [137], 139: [138], 140: [139], 
                           141: [118, 119, 140], 143: [1, 141, 142]},
                           143)],

                    ['(($x >= 5 && $/swapvar < (7 + 3.456 * -0.0)) || !($z[]%d >= 1 && $z[]%d < -9 && $123 == null)) \t && $*! + -234 -        -0 / -9.6)', 
                     FRB.Token("RPAREN", ")", 8, 130)]
                 ]

        for test in tests:
            self._prodId = 0
            self._stateId = 0
            self._nodeId = 0

            inputText = test[0]
            expected = test[1]

            ctx = FRB.ParseContext(inputText, file = file, startLineNo = startLineNo)
            tokens = self._tokenizer.simplifiedMaximalMunch(ctx)

            error = None
            result = None
            try:
                result = self._parser.parse(tokens, ctx = ctx)
            except FRB.SyntaxErr as e:
                error = e

            if (error is not None):
                expected = FRB.SyntaxErr(ctx, expected, process = "parsing")
                self.compareSyntaxErr(error, expected)
            else:
                self.compareParseTree(result, expected)

    # ================================================