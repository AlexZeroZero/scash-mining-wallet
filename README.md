Scash Mining Wallet
===================

Scash Mining Wallet is a Windows Qt wallet based on
[Scash](https://github.com/scashnetwork/scash). It integrates an xmrigCC CPU
mining page directly into the wallet interface.

Features
--------

- Built-in CPU mining controls with selectable pools and thread allocation.
- Chinese and English mining interface.
- Live miner output and purple real-time hashrate display.
- Pool address lookup and SCASH block explorer shortcut.
- Live SCASH/USD price supplied by CoinGecko.
- Default community nodes added to `scash.conf` without overwriting existing
  user settings.
- Bundled miner process-tree shutdown on Windows.

Release package
---------------

Download the Windows package from the repository's Releases page. Keep
`scash-qt-v5.exe` and the `miner` directory together. The miner executable is
loaded from `miner/xmrigDaemon.exe`.

Mining can consume significant CPU resources and electrical power. High
performance mode and locked large memory pages can improve mining efficiency.
Only download release files from a source you trust, and verify the published
SHA-256 checksum.

Third-party software and licensing
----------------------------------

This project preserves the upstream Scash/Bitcoin Core MIT license in
[COPYING](COPYING). The binary release includes xmrigCC under GPLv3 and the
WinRing driver with their respective license notices. See
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

The CoinGecko API is used only for displaying the current SCASH market price.

中文说明
--------

本项目是在 Scash Qt 核心钱包基础上增加 CPU 挖矿功能的一体化钱包。挖矿页面支持
中英文切换、矿池选择、CPU 核心数设置、实时日志与算力、矿池地址查询、区块浏览器
查询和 CoinGecko 实时价格。

挖矿会占用较多 CPU 和电力。建议将电脑设置为高性能模式并开启大锁定内存页。
Windows 发布包中必须保持 `scash-qt-v5.exe` 与 `miner` 文件夹的相对位置不变。

Upstream Bitcoin Core documentation
===================================

https://bitcoincore.org

For an immediately usable, binary version of the Bitcoin Core software, see
https://bitcoincore.org/en/download/.

What is Bitcoin Core?
---------------------

Bitcoin Core connects to the Bitcoin peer-to-peer network to download and fully
validate blocks and transactions. It also includes a wallet and graphical user
interface, which can be optionally built.

Further information about Bitcoin Core is available in the [doc folder](/doc).

License
-------

Bitcoin Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built (see `doc/build-*.md` for instructions) and tested, but it is not guaranteed to be
completely stable. [Tags](https://github.com/bitcoin/bitcoin/tags) are created
regularly from release branches to indicate new official, stable release versions of Bitcoin Core.

The https://github.com/bitcoin-core/gui repository is used exclusively for the
development of the GUI. Its master branch is identical in all monotree
repositories. Release branches and tags do not exist, so please do not fork
that repository unless it is for development reasons.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md)
and useful hints for developers can be found in [doc/developer-notes.md](doc/developer-notes.md).

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](src/test/README.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`. Further details on running
and extending unit tests can be found in [/src/test/README.md](/src/test/README.md).

There are also [regression and integration tests](/test), written
in Python.
These tests can be run (if the [test dependencies](/test) are installed) with: `test/functional/test_runner.py`

The CI (Continuous Integration) systems make sure that every pull request is built for Windows, Linux, and macOS,
and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

Translations
------------

Changes to translations as well as new translations can be submitted to
[Bitcoin Core's Transifex page](https://www.transifex.com/bitcoin/bitcoin/).

Translations are periodically pulled from Transifex and merged into the git repository. See the
[translation process](doc/translation_process.md) for details on how this works.

**Important**: We do not accept translation changes as GitHub pull requests because the next
pull from Transifex would automatically overwrite them again.
