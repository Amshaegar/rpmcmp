# About
rpmcmp is a C++ library with rpmevrcmp and rpmvercmp algorithms made from scratch by found docs and published under the MIT license. MIT license is chosen because I want this library to be able to be used by anyone without limitations - including usage in commercial products.

Documentation that was used for work with RPM versioning and making the rpmevrcmp and rpmvercmp algorithms from scratch:
1. [Fedora Versioning Guidelines](https://docs.fedoraproject.org/en-US/packaging-guidelines/Versioning/)
2. [RPM Versioning](https://rpm-software-management.github.io/rpm/manual/dependencies.html)
3. [Spec file format](https://rpm-software-management.github.io/rpm/manual/spec.html)
3. [rpm.org documentation](https://rpm.org/documentation.html)
4. [Fedora Version Comparison](https://fedoraproject.org/wiki/Archive:Tools/RPM/VersionComparison)
5. [ALT linux rpmvercmp tool doc](https://wiki.altlinux.ru/RPM/rpmvercmp)

Because I didn't found any standard about RPM packages comparison, I've decided to sum up all information that I found on the internet. By information I assume documentation. There is always reference to the [rpmvercmp](https://github.com/rpm-software-management/rpm/blob/master/rpmio/rpmvercmp.cc) function of the RPM in documentation, but it's source code under GNU GPLv2 / GNU LGPLv2 licenses and I would like to publish this library under MIT license - AFAIK I can't rebuild algorithm by this source code as it will fall under section of translation into other language.

![image](https://imgs.xkcd.com/comics/standards.png)

# RPM EVR and vocabulary
We assume that we compare packages - which package is newer/older, so we assume that each package have it EVR.

EVR stands for Epoch Version Release.

E-V-R/EVR name schema: `[Epoch:]Version[-Release]` - Epoch and Version are separated with semicolon symbol `:`, Version and Release separated with a hyphen symbol `-`.

We address to the Version or Release as labels. Label is split into segments according to the rules described in `rpmvercmp algorithm` section.

It's not a guide how to make and set version of your package (for this information please refer to the package versioning guide of your choice). Information, that collected here, is only collected as requirements to RPM versioning and comparing library.

# Comparison

rpmevrcmp stands for RPM EVR Compare

rpmevrcmp stands for RPM Version/Release Compare

## rpmevrcmp algorithm

1. Check if EVR string is valid.
    1. Epoch:
        1. Only a positive integer number are permitted.
        2. Zero epoch is assumed if not provided.
    2. Version/Release:
        1. Any symbol is permitted, except hyphen symbol `-` (hyphen symbol `-` is restricted).
2. Compare the Epoch as non-negative integers (absent Epoch is equal to 0). If equal, then go further.
3. Compare Version with rpmvercmp algorithm. If Version is equal, then compare Release.
4. Compare Release with rpmvercmp algorithm. If Release is equal then packages are equal.

## rpmvercmp algorithm
1. Check for tilde and caret symbols. If they are presented, then:
    1. Tilde symbol `~` before version component means that version with it is older than version without it.  
        E.g. `1.0.0~rc1` < `1.0.0.rc1`.  
        E.g. `1.1~201601` < `1.1`
    2. Caret symbol `^` before version component means that version with it is newer than version without it.  
        E.g. `1.0.0^rc1` > `1.0.0.rc1`.  
        E.g. `1.1^201601` > `1.1`
2. Split label into segments:
    1. Each segment consists of alphanumeric characters (ASCII letters and digits are called alphanumeric characters: digits and letters are defined as ASCII digits (`0`-`9`) and ASCII letters (`a`-`z` and `A`-`Z`) respectively). Other Unicode digits and letters (like accented Latin letters) are not considered as letters. Digits and letters split into separate segments.  
    2. All numbers are converted to their numeric value without leading zeroes: `10` -> `10`, `000230` -> `230`, `00000` -> `0`.  
    E.g. `1.002.3.abc.001ab` -> [`1`,`2`,`3`,`abc`,`1`,ab].
3. Each segment compared in order.
    1. If one of the elements is a number, while the other is alphabetic, the numeric elements is considered newer.  
    E.g. `10` > `abc`, `0` > `Z`.
    2. If both the elements are alphabetic, they are compared using the Unix strcmp function, with the greater string resulting in a newer element. If the strings are identical, the elements are decided equal.  
    E.g. `b` > `a`, `add` > `ZULU` , because lowercase characters win in strcmp comparisons, `aba` > `ab`, `aaa` == `aaa`.
    3. In case one of the lists of segments runs out, the other label wins as the newer label.  
    E.g. [`1`, `2`, `0`] > [`1`, `2`].

**NOTE**

`Each segment is then compared in order with the right most segment being the least significant. ` from [RPM Versioning](https://rpm-software-management.github.io/rpm/manual/dependencies.html) is interpreted as `If the elements are decided to be equal, the next elements are compared until we either reach different elements or one of the lists runs out. In case one of the lists run out, the other label wins as the newer label. So, for example, (1, 2) is newer than (1, 1), and (1, 2, 0) is newer than (1, 2).` from [Fedora Version Comparison](https://fedoraproject.org/wiki/Archive:Tools/RPM/VersionComparison).

**NOTE**

There is a contradiction between algorithms described in [RPM Versioning](https://rpm-software-management.github.io/rpm/manual/dependencies.html) and [Fedora Version Comparison](https://fedoraproject.org/wiki/Archive:Tools/RPM/VersionComparison) - there is an addition in [RPM Versioning](https://rpm-software-management.github.io/rpm/manual/dependencies.html): `The digit segments strip leading zeroes and compare the strlen before doing a strcmp: if both numerical strings are equal, the longer string is larger.` As far as that seems not very logical to me and I can't find any standard or another source of information about this algorithm that can be canonical, I've decided not to include this logic in current algorithm. If you can support me with such standard, I'll be very thankful to you.


**NOTE** from [Fedora Version Comparison](https://fedoraproject.org/wiki/Archive:Tools/RPM/VersionComparison):
```
Please note that the algorithm's actions is undefined in some cases, in a ways may make the resulting comparisons stop working sanely (see https://bugzilla.redhat.com/bugzilla/show_bug.cgi?id=178798 for an example where the order of the comparison is more important than the operands). To avoid these, make sure that all your labels start and end with alphanumeric characters. So while things like "1.", "+a", or "_" are allowed as labels, the result of such comparisons are undefined. For the exact (non-symmetric) algorithm, see lib/vercmp.c in the RPM source code. The following algorithm is a simplification based on the version available in FC4, and is considered to be stable, as the last time it changed in any way was in January 2003. 
```

**NOTE**

There is  note in [Fedora Versioning Guidelines](https://docs.fedoraproject.org/en-US/packaging-guidelines/Versioning/): `Note that 0.4.1^<something> sorts higher than 0.4.1, but lower than both 0.4.2 and 0.4.1.<anything>.` I don't understand why `0.4.1^<something>` < `0.4.1.<anything>`, so it's not supported in the library.

# Library usage examples
C++ style.  
Make objects and compare them:
```cpp
auto versionA = rpmcmplib::RpmEvr("3.0.0.fc");
auto versionB = rpmcmplib::RpmEvr("3.0.0_fc");
bool result = versionA == versionB;
```
or just:
```cpp
bool result = rpmcmplib::RpmEvr("3.0.0.fc") == rpmcmplib::RpmEvr("3.0.0_fc");
```

C style.  
If for some reason you have to or want to use this library similar to rpmvercmp/rpmevrcmp function from rpmlib, then you can use it in a such way:
```cpp
int result2 = rpmcmplib::RpmVer::cmp("0.5.0.1", "0.5.0.post1");
int result1 = rpmcmplib::RpmEvr::cmp("0.5.0.post1", "0.5.0.1");
```
**! But beware** that current library is checking input values for validity and in case of invalid values will throw an exception. To prevent this - check values for validity by yourself before comparison:
```cpp
std::string versionA = "0.5.0.post1";
std::string versionB = "0.5.0.1";

std::string isVersionValid = rpmcmplib::RpmVer::isValid(versionA);
if (!isVersionValid.empty()) {
    // handle error: reason of invalidity is in isVersionValid
}

isVersionValid = rpmcmplib::RpmVer::isValid(versionB);
if (!isVersionValid.empty()) {
    // handle error: reason of invalidity is in isVersionValid
}

int result = rpmcmplib::RpmVer::cmp(versionA, versionB);
```

For more examples of library usage see tests.

# Plans and TODOs
1. Add this library to the Conan and vcpkg.
2. Rewrite all the tests in data-driven manner.

# Legal notice
As far as this readme was build using Fedora documentation that available under CC BY-SA 4.0, this README is also is available under [CC BY-SA 4.0](http://creativecommons.org/licenses/by-sa/4.0/legalcode).