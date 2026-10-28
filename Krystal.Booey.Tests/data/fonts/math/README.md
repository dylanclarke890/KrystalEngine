# Math test fonts

From web-platform-tests (https://github.com/web-platform-tests/wpt, `fonts/math/`, as imported by WebKit at `300e80c70fd3`), unmodified, under its 3-clause BSD licence (`LICENSE.md`). Each is made by a script in WPT's `mathml/tools/` with a 1000-unit em and one MATH table value set, as its name says:

- `axisheight5000-verticalarrow14000.woff` (`axisheight.py`): `AxisHeight` 5000. SHA-256 `5fb082cf27e4375ee4a1ccff09baca2915217fd4555a512f88ff5ec61868c745`.
- `fraction-rulethickness10000.woff` (`fractions.py`): `FractionRuleThickness` 10000. SHA-256 `4fecc416fc56d6c0d1d1b4456edf9d8a77f3f8795f5866e1b25a0f1c81f99367`.
- `largeop-displayoperatorminheight2000-2AFF-italiccorrection3000.woff` (`largeop.py`): `DisplayOperatorMinHeight` 2000; U+2AFF has a display size variant with an italic correction of 3000. SHA-256 `e39a2d2ffcf5f52d330dffa5418495845f6496d2a19dd8e185f227f9f0a98673`.
- `radical-degreebottomraisepercent25-rulethickness1000.woff` (`radicals.py`): `RadicalDegreeBottomRaisePercent` 25, `RadicalRuleThickness` 1000. SHA-256 `079c679066fb06c71e5ecac1652c16a86795b8680af44bdbe66963df45c1a63a`.
- `scriptpercentscaledown80-scriptscriptpercentscaledown40.woff` (`percentscaledown.py`): `ScriptPercentScaleDown` 80, `ScriptScriptPercentScaleDown` 40. SHA-256 `5a4c1cbfdbba9195e6d1676ebe8a9c85b7da538abfdba47a192753e177ef5eae`.
- `stretchy.woff` (`stretchy.py`): U+295A and U+295C stretch both ways through four size variants (`v0` to `v3` at U+E000, U+E002, U+E004, U+E006, `h0` to `h3` at U+E001 to U+E007) and an assembly of a part and an extender (`v2` then `v1`, `h2` then `h1`). SHA-256 `02462f896bceecaf2d4f96e9cda795c81aee4452cdf993d0566d29ab79245fb3`.
