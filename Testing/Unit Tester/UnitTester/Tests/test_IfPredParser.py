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
                     {3: [2, 1]},
                     3)],

                    ["$swapvar == 5", 
                     FRB.ParseTree({
                        1: FRB.ParseNode(1, token=FRB.Token('STARTTOKEN', 'STARTTOKEN', 8, 0)),
                        2: FRB.ParseNode(2, token=FRB.Token('ID', '$swapvar', 8, 1)),
                        3: FRB.ParseNode(3, prodId='variable'),
                        4: FRB.ParseNode(4, prodId='expr reduce'),
                        5: FRB.ParseNode(5, prodId='keyexpr reduce'),
                        6: FRB.ParseNode(6, token=FRB.Token('EQ', '==', 8, 10)),
                        7: FRB.ParseNode(7, token=FRB.Token('INT', '5', 8, 13)),
                        8: FRB.ParseNode(8, prodId='int'),
                        9: FRB.ParseNode(9, prodId='expr reduce'),
                        10: FRB.ParseNode(10, prodId='keyexpr reduce'),
                        11: FRB.ParseNode(11, prodId='eq'),
                        12: FRB.ParseNode(12, prodId='ntest reduce'),
                        13: FRB.ParseNode(13, prodId='pred reduce'),
                        14: FRB.ParseNode(14, token=FRB.Token('ENDTOKEN', 'ENDTOKEN', 9, 0)),
                        15: FRB.ParseNode(15, prodId='start'),
                    },
                    {3: [2], 4: [3], 5: [4], 8: [7], 9: [8], 10: [9], 11: [10, 6, 5], 12: [11], 13: [12], 15: [14, 13, 1]},
                    15)],

                    ['$x >= 5', 
                     FRB.ParseTree({
                        1: FRB.ParseNode(1, token=FRB.Token('STARTTOKEN', 'STARTTOKEN', 8, 0)),
                        2: FRB.ParseNode(2, token=FRB.Token('ID', '$x', 8, 1)),
                        3: FRB.ParseNode(3, prodId='variable'),
                        4: FRB.ParseNode(4, prodId='expr reduce'),
                        5: FRB.ParseNode(5, token=FRB.Token('GE', '>=', 8, 4)),
                        6: FRB.ParseNode(6, token=FRB.Token('INT', '5', 8, 7)),
                        7: FRB.ParseNode(7, prodId='int'),
                        8: FRB.ParseNode(8, prodId='expr reduce'),
                        9: FRB.ParseNode(9, prodId='ge'),
                        10: FRB.ParseNode(10, prodId='ntest reduce'),
                        11: FRB.ParseNode(11, prodId='pred reduce'),
                        12: FRB.ParseNode(12, token=FRB.Token('ENDTOKEN', 'ENDTOKEN', 9, 0)),
                        13: FRB.ParseNode(13, prodId='start'),
                     }, {3: [2], 4: [3], 7: [6], 8: [7], 9: [8, 5, 4], 10: [9], 11: [10], 13: [12, 11, 1]},
                     13)],

                     ['($/swapvar < 7)', 
                      FRB.ParseTree({
                        1: FRB.ParseNode(1, token=FRB.Token('STARTTOKEN', 'STARTTOKEN', 8, 0)),
                        2: FRB.ParseNode(2, token=FRB.Token('LPAREN', '(', 8, 1)),
                        3: FRB.ParseNode(3, token=FRB.Token('ID', '$/swapvar', 8, 2)),
                        4: FRB.ParseNode(4, prodId='variable'),
                        5: FRB.ParseNode(5, prodId='expr reduce'),
                        6: FRB.ParseNode(6, token=FRB.Token('LT', '<', 8, 12)),
                        7: FRB.ParseNode(7, token=FRB.Token('INT', '7', 8, 14)),
                        8: FRB.ParseNode(8, prodId='int'),
                        9: FRB.ParseNode(9, prodId='expr reduce'),
                        10: FRB.ParseNode(10, prodId='lt'),
                        11: FRB.ParseNode(11, prodId='ntest reduce'),
                        12: FRB.ParseNode(12, prodId='pred reduce'),
                        13: FRB.ParseNode(13, token=FRB.Token('RPAREN', ')', 8, 15)),
                        14: FRB.ParseNode(14, prodId='bracket loop'),
                        15: FRB.ParseNode(15, prodId='expr reduce'),
                        16: FRB.ParseNode(16, prodId='keyexpr reduce'),
                        17: FRB.ParseNode(17, prodId='test reduce'),
                        18: FRB.ParseNode(18, prodId='ntest reduce'),
                        19: FRB.ParseNode(19, prodId='pred reduce'),
                        20: FRB.ParseNode(20, token=FRB.Token('ENDTOKEN', 'ENDTOKEN', 9, 0)),
                        21: FRB.ParseNode(21, prodId='start')
                      }, {4: [3], 5: [4], 8: [7], 9: [8], 10: [9, 6, 5], 11: [10], 12: [11], 14: [13, 12, 2], 15: [14], 16: [15], 17: [16], 18: [17], 19: [18], 21: [20, 19, 1]},
                      21)],

                      ['(($x >= 5 && $/swapvar < (7 + 3.456 * -0.0)) || !($z[]%d >= "hello world" && $z[]%d < \'goodbye world\' && $123 == null)) \t && $*! % -234 -        -0 / -9.6',
                       FRB.ParseTree({
                            1: FRB.ParseNode(1, token=FRB.Token('STARTTOKEN', 'STARTTOKEN', 8, 0)),
                            2: FRB.ParseNode(2, token=FRB.Token('LPAREN', '(', 8, 1)),
                            3: FRB.ParseNode(3, token=FRB.Token('LPAREN', '(', 8, 2)),
                            4: FRB.ParseNode(4, token=FRB.Token('ID', '$x', 8, 3)),
                            5: FRB.ParseNode(5, prodId='variable'),
                            6: FRB.ParseNode(6, prodId='expr reduce'),
                            7: FRB.ParseNode(7, token=FRB.Token('GE', '>=', 8, 6)),
                            8: FRB.ParseNode(8, token=FRB.Token('INT', '5', 8, 9)),
                            9: FRB.ParseNode(9, prodId='int'),
                            10: FRB.ParseNode(10, prodId='expr reduce'),
                            11: FRB.ParseNode(11, prodId='ge'),
                            12: FRB.ParseNode(12, prodId='ntest reduce'),
                            13: FRB.ParseNode(13, prodId='pred reduce'),
                            14: FRB.ParseNode(14, token=FRB.Token('AND', '&&', 8, 11)),
                            15: FRB.ParseNode(15, token=FRB.Token('ID', '$/swapvar', 8, 14)),
                            16: FRB.ParseNode(16, prodId='variable'),
                            17: FRB.ParseNode(17, prodId='expr reduce'),
                            18: FRB.ParseNode(18, token=FRB.Token('LT', '<', 8, 24)),
                            19: FRB.ParseNode(19, token=FRB.Token('LPAREN', '(', 8, 26)),
                            20: FRB.ParseNode(20, token=FRB.Token('INT', '7', 8, 27)),
                            21: FRB.ParseNode(21, prodId='int'),
                            22: FRB.ParseNode(22, prodId='expr reduce'),
                            23: FRB.ParseNode(23, token=FRB.Token('PLUS', '+', 8, 29)),
                            24: FRB.ParseNode(24, token=FRB.Token('FLOAT', '3.456', 8, 31)),
                            25: FRB.ParseNode(25, prodId='float'),
                            26: FRB.ParseNode(26, prodId='add'),
                            27: FRB.ParseNode(27, token=FRB.Token('STAR', '*', 8, 37)),
                            28: FRB.ParseNode(28, token=FRB.Token('FLOAT', '-0.0', 8, 39)),
                            29: FRB.ParseNode(29, prodId='float'),
                            30: FRB.ParseNode(30, prodId='multiply'),
                            31: FRB.ParseNode(31, prodId='keyexpr reduce'),
                            32: FRB.ParseNode(32, prodId='test reduce'),
                            33: FRB.ParseNode(33, prodId='ntest reduce'),
                            34: FRB.ParseNode(34, prodId='pred reduce'),
                            35: FRB.ParseNode(35, token=FRB.Token('RPAREN', ')', 8, 43)),
                            36: FRB.ParseNode(36, prodId='bracket loop'),
                            37: FRB.ParseNode(37, prodId='expr reduce'),
                            38: FRB.ParseNode(38, prodId='lt'),
                            39: FRB.ParseNode(39, prodId='ntest reduce'),
                            40: FRB.ParseNode(40, prodId='and'),
                            41: FRB.ParseNode(41, token=FRB.Token('RPAREN', ')', 8, 44)),
                            42: FRB.ParseNode(42, prodId='bracket loop'),
                            43: FRB.ParseNode(43, prodId='expr reduce'),
                            44: FRB.ParseNode(44, prodId='keyexpr reduce'),
                            45: FRB.ParseNode(45, prodId='test reduce'),
                            46: FRB.ParseNode(46, prodId='ntest reduce'),
                            47: FRB.ParseNode(47, prodId='pred reduce'),
                            48: FRB.ParseNode(48, token=FRB.Token('OR', '||', 8, 46)),
                            49: FRB.ParseNode(49, token=FRB.Token('NOT', '!', 8, 49)),
                            50: FRB.ParseNode(50, token=FRB.Token('LPAREN', '(', 8, 50)),
                            51: FRB.ParseNode(51, token=FRB.Token('ID', '$z[]%d', 8, 51)),
                            52: FRB.ParseNode(52, prodId='variable'),
                            53: FRB.ParseNode(53, prodId='expr reduce'),
                            54: FRB.ParseNode(54, token=FRB.Token('GE', '>=', 8, 58)),
                            55: FRB.ParseNode(55, token=FRB.Token('STRING', '"hello world"', 8, 61)),
                            56: FRB.ParseNode(56, prodId='string'),
                            57: FRB.ParseNode(57, prodId='expr reduce'),
                            58: FRB.ParseNode(58, prodId='ge'),
                            59: FRB.ParseNode(59, prodId='ntest reduce'),
                            60: FRB.ParseNode(60, prodId='pred reduce'),
                            61: FRB.ParseNode(61, token=FRB.Token('AND', '&&', 8, 75)),
                            62: FRB.ParseNode(62, token=FRB.Token('ID', '$z[]%d', 8, 78)),
                            63: FRB.ParseNode(63, prodId='variable'),
                            64: FRB.ParseNode(64, prodId='expr reduce'),
                            65: FRB.ParseNode(65, token=FRB.Token('LT', '<', 8, 85)),
                            66: FRB.ParseNode(66, token=FRB.Token('STRING', "'goodbye world'", 8, 87)),
                            67: FRB.ParseNode(67, prodId='string'),
                            68: FRB.ParseNode(68, prodId='expr reduce'),
                            69: FRB.ParseNode(69, prodId='lt'),
                            70: FRB.ParseNode(70, prodId='ntest reduce'),
                            71: FRB.ParseNode(71, prodId='and'),
                            72: FRB.ParseNode(72, token=FRB.Token('AND', '&&', 8, 103)),
                            73: FRB.ParseNode(73, token=FRB.Token('ID', '$123', 8, 106)),
                            74: FRB.ParseNode(74, prodId='variable'),
                            75: FRB.ParseNode(75, prodId='expr reduce'),
                            76: FRB.ParseNode(76, prodId='keyexpr reduce'),
                            77: FRB.ParseNode(77, token=FRB.Token('EQ', '==', 8, 111)),
                            78: FRB.ParseNode(78, token=FRB.Token('NULL', 'null', 8, 114)),
                            79: FRB.ParseNode(79, prodId='null'),
                            80: FRB.ParseNode(80, prodId='eq'),
                            81: FRB.ParseNode(81, prodId='ntest reduce'),
                            82: FRB.ParseNode(82, prodId='and'),
                            83: FRB.ParseNode(83, token=FRB.Token('RPAREN', ')', 8, 118)),
                            84: FRB.ParseNode(84, prodId='bracket loop'),
                            85: FRB.ParseNode(85, prodId='expr reduce'),
                            86: FRB.ParseNode(86, prodId='keyexpr reduce'),
                            87: FRB.ParseNode(87, prodId='test reduce'),
                            88: FRB.ParseNode(88, prodId='not'),
                            89: FRB.ParseNode(89, prodId='or'),
                            90: FRB.ParseNode(90, token=FRB.Token('RPAREN', ')', 8, 119)),
                            91: FRB.ParseNode(91, prodId='bracket loop'),
                            92: FRB.ParseNode(92, prodId='expr reduce'),
                            93: FRB.ParseNode(93, prodId='keyexpr reduce'),
                            94: FRB.ParseNode(94, prodId='test reduce'),
                            95: FRB.ParseNode(95, prodId='ntest reduce'),
                            96: FRB.ParseNode(96, prodId='pred reduce'),
                            97: FRB.ParseNode(97, token=FRB.Token('AND', '&&', 8, 123)),
                            98: FRB.ParseNode(98, token=FRB.Token('ID', '$*!', 8, 126)),
                            99: FRB.ParseNode(99, prodId='variable'),
                            100: FRB.ParseNode(100, prodId='expr reduce'),
                            101: FRB.ParseNode(101, token=FRB.Token('PCT', '%', 8, 130)),
                            102: FRB.ParseNode(102, token=FRB.Token('INT', '-234', 8, 132)),
                            103: FRB.ParseNode(103, prodId='int'),
                            104: FRB.ParseNode(104, prodId='modulus'),
                            105: FRB.ParseNode(105, token=FRB.Token('MINUS', '-', 8, 137)),
                            106: FRB.ParseNode(106, token=FRB.Token('INT', '-0', 8, 146)),
                            107: FRB.ParseNode(107, prodId='int'),
                            108: FRB.ParseNode(108, prodId='subtract'),
                            109: FRB.ParseNode(109, token=FRB.Token('SLASH', '/', 8, 149)),
                            110: FRB.ParseNode(110, token=FRB.Token('FLOAT', '-9.6', 8, 151)),
                            111: FRB.ParseNode(111, prodId='float'),
                            112: FRB.ParseNode(112, prodId='divide'),
                            113: FRB.ParseNode(113, prodId='keyexpr reduce'),
                            114: FRB.ParseNode(114, prodId='test reduce'),
                            115: FRB.ParseNode(115, prodId='ntest reduce'),
                            116: FRB.ParseNode(116, prodId='and'),
                            117: FRB.ParseNode(117, token=FRB.Token('ENDTOKEN', 'ENDTOKEN', 9, 0)),
                            118: FRB.ParseNode(118, prodId='start')
                       }, {5: [4], 6: [5], 9: [8], 10: [9], 11: [10, 7, 6], 12: [11], 13: [12], 16: [15], 17: [16], 21: [20], 22: [21], 25: [24], 
                           26: [25, 23, 22], 29: [28], 30: [29, 27, 26], 31: [30], 32: [31], 33: [32], 34: [33], 36: [35, 34, 19], 37: [36], 
                           38: [37, 18, 17], 39: [38], 40: [39, 14, 13], 42: [41, 40, 3], 43: [42], 44: [43], 45: [44], 46: [45], 47: [46], 
                           52: [51], 53: [52], 56: [55], 57: [56], 58: [57, 54, 53], 59: [58], 60: [59], 63: [62], 64: [63], 67: [66], 68: [67], 
                           69: [68, 65, 64], 70: [69], 71: [70, 61, 60], 74: [73], 75: [74], 76: [75], 79: [78], 80: [79, 77, 76], 81: [80], 
                           82: [81, 72, 71], 84: [83, 82, 50], 85: [84], 86: [85], 87: [86], 88: [87, 49], 89: [88, 48, 47], 91: [90, 89, 2], 
                           92: [91], 93: [92], 94: [93], 95: [94], 96: [95], 99: [98], 100: [99], 103: [102], 104: [103, 101, 100], 107: [106], 
                           108: [107, 105, 104], 111: [110], 112: [111, 109, 108], 113: [112], 114: [113], 115: [114], 116: [115, 97, 96], 118: [117, 116, 1]},
                           118)],

                    ['(($x >= 5 && $/swapvar < (7 + 3.456 * -0.0)) || !($z[]%d >= "hello world" && $z[]%d < \'goodbye world\' && $123 == null)) \t && $*! % -234 -        -0 / -9.6)', 
                     FRB.Token("RPAREN", ")", 8, 155)]
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