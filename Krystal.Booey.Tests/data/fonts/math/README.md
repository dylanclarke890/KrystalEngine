# Math test fonts

From web-platform-tests (https://github.com/web-platform-tests/wpt, `fonts/math/`, as imported by WebKit at `300e80c70fd3`), unmodified, under its 3-clause BSD licence (`LICENSE.md`). Each is made by a script in WPT's `mathml/tools/` with a 1000-unit em and one MATH table value set, as its name says:

- `axisheight5000-verticalarrow14000.woff` (`axisheight.py`): `AxisHeight` 5000. SHA-256 `5fb082cf27e4375ee4a1ccff09baca2915217fd4555a512f88ff5ec61868c745`.
- `fraction-axisheight7000-rulethickness1000.woff` (`fractions.py`): `AxisHeight` 7000, `FractionRuleThickness` 1000. SHA-256 `959aa71732b7ede5ff66f74c12e254b0c51250fed955f2b5755fafb2d47a2315`.
- `fraction-denominatorgapmin4000-rulethickness1000.woff` (`fractions.py`): `FractionDenominatorGapMin` 4000, `FractionRuleThickness` 1000. SHA-256 `cc30569a9fdabc181a0490faf3ec49cc76bd028a1f91cb8fdeb1261d804f5e5d`.
- `fraction-numeratordisplaystylegapmin8000-rulethickness1000.woff` (`fractions.py`): `FractionNumDisplayStyleGapMin` 8000, `FractionRuleThickness` 1000. SHA-256 `b1ad5df3eb647ec7d7167bd5f82686adc2b74cce5c3af085343653681186a892`.
- `fraction-numeratorshiftup11000-axisheight1000-rulethickness1000.woff` (`fractions.py`): `FractionNumeratorShiftUp` 11000, `AxisHeight` 1000, `FractionRuleThickness` 1000. SHA-256 `00c6894fe374e9d5f774702aceb0ef2d9cd5810e7d31447e29a4c655beb46f86`.
- `fraction-rulethickness10000.woff` (`fractions.py`): `FractionRuleThickness` 10000. SHA-256 `4fecc416fc56d6c0d1d1b4456edf9d8a77f3f8795f5866e1b25a0f1c81f99367`.
- `largeop-displayoperatorminheight2000-2AFF-italiccorrection3000.woff` (`largeop.py`): `DisplayOperatorMinHeight` 2000; U+2AFF has a display size variant with an italic correction of 3000. SHA-256 `e39a2d2ffcf5f52d330dffa5418495845f6496d2a19dd8e185f227f9f0a98673`.
- `limits-lowerlimitgapmin11000.woff` (`limits.py`): `LowerLimitGapMin` 11000, with a large operator U+2211. SHA-256 `3c4342c7a19b6baf74de4246db00b790a3e39706a97038b7a2edce2456b5d448`.
- `limits-upperlimitgapmin7000.woff` (`limits.py`): `UpperLimitGapMin` 7000, with a large operator U+2211. SHA-256 `89da743eb0e88cf49e1d4670e81556b5c9b561a828c41a441adb2b76641bcec6`.
- `radical-degreebottomraisepercent25-rulethickness1000.woff` (`radicals.py`): `RadicalDegreeBottomRaisePercent` 25, `RadicalRuleThickness` 1000. SHA-256 `079c679066fb06c71e5ecac1652c16a86795b8680af44bdbe66963df45c1a63a`.
- `radical-extraascender3000-rulethickness1000.woff` (`radicals.py`): `RadicalExtraAscender` 3000, `RadicalRuleThickness` 1000. SHA-256 `c1c9d5c5c71b0bc464c2ae33efe53e38e281c41ab77ffb73a4bb221ea724860d`.
- `radical-kernbeforedegree4000-rulethickness1000.woff` (`radicals.py`): `RadicalKernBeforeDegree` 4000, `RadicalRuleThickness` 1000. SHA-256 `292d99e8f4e7aaed575a4b6c88757066bfad504a449d5f5eeef649265c86d2a3`.
- `radical-verticalgap6000-rulethickness1000.woff` (`radicals.py`): `RadicalVerticalGap` 6000, `RadicalRuleThickness` 1000. SHA-256 `06ce1a2ab01e509c0838b2114a9a047d19ad3d175827e9e12e8fed0fc6892175`.
- `scriptpercentscaledown80-scriptscriptpercentscaledown40.woff` (`percentscaledown.py`): `ScriptPercentScaleDown` 80, `ScriptScriptPercentScaleDown` 40. SHA-256 `5a4c1cbfdbba9195e6d1676ebe8a9c85b7da538abfdba47a192753e177ef5eae`.
- `scripts-spaceafterscript3000.woff` (`scripts.py`): `SpaceAfterScript` 3000. SHA-256 `5266f9063424a4dee7aac2afa157d16de154dc1523622990ed6e736f55764a49`.
- `scripts-subscriptshiftdown6000.woff` (`scripts.py`): `SubscriptShiftDown` 6000. SHA-256 `dee32822fc76b0cce71b745138d871e415f79400c9725028da64fb6775b2ed1c`.
- `scripts-subsuperscriptgapmin11000.woff` (`scripts.py`): `SubSuperscriptGapMin` 11000. SHA-256 `8d51b89f65c3feb3b721e85813c3a51db2b56ae9798612887c70c39ed67aeb06`.
- `scripts-superscriptshiftup7000.woff` (`scripts.py`): `SuperscriptShiftUp` 7000. SHA-256 `13f174866bf62592db3f217cbd2e48f349f84b1c4b1446e81a337ef4b70b2fb4`.
- `stack-gapmin8000.woff` (`stacks.py`): `StackGapMin` 8000. SHA-256 `e96d0b9bc63d992b77ad533618af51ffd7d445b61ca003df633931e5bf16ca04`.
- `stack-topshiftup9000-axisheight1000.woff` (`stacks.py`): `StackTopShiftUp` 9000, `AxisHeight` 1000. SHA-256 `696bc6e2b8ade44dc8f629b3f5a91a848aef2b358bf34030448ae3797fcc5de2`.
- `stretchy.woff` (`stretchy.py`): U+295A and U+295C stretch both ways through four size variants (`v0` to `v3` at U+E000, U+E002, U+E004, U+E006, `h0` to `h3` at U+E001 to U+E007) and an assembly of a part and an extender (`v2` then `v1`, `h2` then `h1`). SHA-256 `02462f896bceecaf2d4f96e9cda795c81aee4452cdf993d0566d29ab79245fb3`.
- `underover-accentbaseheight4000-overbarverticalgap11000.woff` (`underover.py`): `AccentBaseHeight` 4000, `OverbarVerticalGap` 11000. SHA-256 `5b1f26309bd4f7665beaec12d91c2504675a51db8e9809133cb12cfbd4e4df6c`.
