# AI Agent Help Instructions

<a href="https://claude.ai/login"><img align="top" src="https://img.shields.io/badge/Claude-d97757?style=for-the-badge&logo=claude&logoColor=white" alt="Claude"></a>
<img align="top" src="../Docs/src/_static/images/TheCouncilofClaudeAgentsBadgeWithCount.svg" alt="The Council of CLAUDE Agents">


<br>

> [!NOTE]
> Instructions were editted (and **only editted**) by:
> 
> ![⚔🗡The Council of CLAUDE agents🗡⚔](../Docs/src/_static/images/TheCouncilOfClaudeAgents.svg)

<br>
Some instructions to help AI coding agents learn how to operate and build the project

<br>
<br>

## Council Members

Special Thanks to ❤:

- ![Static Badge](https://img.shields.io/badge/%F0%9F%A5%87%F0%9F%8F%97%EF%B8%8F%20The%20Founding%20Architect-1-%23eab308?style=for-the-badge&labelColor=%231e293b)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%8E%A8%E2%99%BB%EF%B8%8F%20The%20Colour%20Migrator-1-%23db2777?style=flat-square&labelColor=%234c1d95)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%94%90%E2%9A%92%EF%B8%8F%20The%20Hash%20Forger-1-%230d9488?style=plastic&labelColor=%23134e4a)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%90%8D%F0%9F%94%A8%20The%20Cython%20Pioneer-1-%233776ab?style=flat&labelColor=%230f172a)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%95%B8%EF%B8%8F%F0%9F%94%81%20The%20Cycle%20Tamer-1-%234f46e5?style=for-the-badge&labelColor=%231e1b4b)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%A7%B5%F0%9F%94%A4%20The%20Tokenizer%20Weaver-2-%23f59e0b?style=for-the-badge&labelColor=%236d28d9)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%94%99%F0%9F%A7%B1%20The%20Back--Filler-1-%23c2410c?style=social&labelColor=%231c1917)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%A7%9F%F0%9F%94%AA%20The%20Dangling%20Reference%20Slayer-1-%23dc2626?style=for-the-badge&labelColor=%2318181b)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%9B%A1%EF%B8%8F%F0%9F%94%A8%20The%20Sigil%20Smith-1-%237c3aed?style=for-the-badge&labelColor=%232b1220)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%97%83%EF%B8%8F%F0%9F%94%AE%20The%20Asset%20Alchemist-1-%23a855f7?style=for-the-badge&labelColor=%2378350f)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%93%9C%E2%9A%99%EF%B8%8F%20The%20Grammar%20Grafter-1-%23059669?style=for-the-badge&labelColor=%23052e16)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%94%AE%E2%9A%96%EF%B8%8F%20The%20Z3%20Oracle-1-%230891b2?style=for-the-badge&labelColor=%23164e63)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%8C%89%F0%9F%94%80%20The%20Predicate%20Reparenter-1-%232563eb?style=for-the-badge&labelColor=%231e3a8a)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%8C%90%F0%9F%94%92%20The%20Graph%20Keeper-1-%230284c7?style=for-the-badge&labelColor=%230c4a6e)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%8E%9E%EF%B8%8F%F0%9F%A7%AC%20The%20Frame%20Decoder-1-%2322c55e?style=flat-square&labelColor=%230f172a)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%96%BC%EF%B8%8F%F0%9F%A7%B5%20The%20Texture%20Weaver-1-%2384cc16?style=for-the-badge&labelColor=%231a2e05)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%A7%A9%F0%9F%94%93%20The%20Classifier%20Unlocker-1-%23f43f5e?style=flat-square&labelColor=%233f3f46)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%95%B5%EF%B8%8F%F0%9F%93%A6%20The%20DLL%20Detective-1-%23f97316?style=flat&labelColor=%23431407)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%94%A9%F0%9F%93%90%20The%20Register%20Templater-1-%23b45309?style=plastic&labelColor=%23292524)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%95%B8%EF%B8%8F%F0%9F%96%8C%EF%B8%8F%20The%20Graph%20Edit%20Colourist-1-%23d946ef?style=flat-square&labelColor=%233b0764)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%8F%AD%F0%9F%97%93%EF%B8%8F%20The%20Strategy%20Foundry-1-%23be123c?style=plastic&labelColor=%23450a0a)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%90%A7%F0%9F%A7%AD%20The%20Penguin%20Pathfinder-1-%230ea5e9?style=flat&labelColor=%23082f49)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%A7%B9%F0%9F%94%97%20The%20Reachability%20Sweeper-1-%2365a30d?style=for-the-badge&labelColor=%2314532d)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%A7%A9%F0%9F%94%8C%20The%20Context%20Seamsmith-1-%239333ea?style=flat-square&labelColor=%232e1065)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%93%A3%F0%9F%AA%9E%20The%20View%20Herald-1-%23facc15?style=flat&labelColor=%230c4a6e)
- ![Static Badge](https://img.shields.io/badge/%F0%9F%94%A4%F0%9F%A7%BF%20The%20Grapheme%20Warden-1-%2314b8a6?style=flat-square&labelColor=%23042f2e)
