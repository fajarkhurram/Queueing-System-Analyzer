# Queueing System Analyzer — Deficiencies Found & Fixed

This document lists every bug found in the original project, why it was
wrong, and how it was fixed. Useful for viva Q&A.

## 1. M/M/S — wrong utilization formula (critical)

**File:** `MMS.cpp`

**Before:** `utilization = arrivalRate / serviceRate;`

This is the *offered load* `a = λ/μ`, not the utilization. For a
multi-server queue, utilization (the fraction of time each server is
busy) is:

```
ρ = λ / (s·μ)
```

Using `λ/μ` instead of `λ/(s·μ)` meant the system was reported "unstable"
far too often (e.g. λ=10, μ=4, s=3 gives λ/μ=2.5 ≥ 1, wrongly flagged
unstable, even though the true ρ = 2.5/3 = 0.833 is perfectly stable),
and the server-busy visualization showed the wrong number of busy
servers.

**Fix:** compute `ρ = λ/(s·μ)`, and use the already-defined but
previously-unused `InputValidator::isMultiServerStable()` for the
stability check, matching M/M/1's use of `isSingleServerStable()`.

## 2. M/G/S and G/G/S — Lq/Wq computed in the wrong order (critical)

**Files:** `MGS.cpp`, `GGS.cpp`

Both use the Sakasegawa/Allen-Cunneen approximation, which produces the
**waiting time Wq** (a value in time units):

```
Wq ≈ [(Ca² + Cs²)/2] · [ρ^(√(2(s+1))−1) / (s(1−ρ))] · (1/μ)
```

`Lq` must then come from Little's Law: `Lq = λ·Wq`.

**Before:** both files skipped the `(1/μ)` step and used the
dimensionless bracket directly as `Lq`, then derived `Wq = Lq/λ`
backwards. This is dimensionally wrong and silently drops a factor of
`s` (number of servers) for every case with `s > 1` — it only ever
looked correct by coincidence when `s = 1`.

**Verified fix:** with λ=10, μ=4, s=3 and Cs²=1 (i.e. reducing to M/M/S),
the corrected G/M/S-style approximation now gives `Lq ≈ 3.58` against
the exact M/M/S value of `Lq ≈ 3.51` — a small, expected approximation
error, instead of the wildly wrong values the old code produced.

**Fix:** compute `Wq` first (with the `(1/μ)` factor), then
`Lq = λ·Wq`, exactly matching the pattern `GG1.cpp` already used
correctly.

## 3. M/G/1 — missing input validation

**File:** `MG1.cpp`

Only checked `serviceTimeVariance`; `arrivalRate`/`serviceRate` were
never checked for positivity, which is a division-by-zero / negative-
result risk if this class is ever called from anywhere besides the
already-validated UI (e.g. unit tests, another caller).

**Fix:** added `InputValidator::isPositive()` checks for both rates.

## 4. M/G/S — off-by-one validity check allowed λ = 0

**File:** `MGS.cpp`

**Before:** `arrivalRate < 0` (only rejects negative, not zero).
With `arrivalRate == 0`, later code divides by `arrivalRate`,
producing `0/0 = NaN`.

**Fix:** changed to `arrivalRate <= 0`.

## 5. CMakeLists.txt — Qt version requirement too strict

**Before:** `find_package(Qt6 6.5 REQUIRED ...)` and an unconditional
call to `qt_generate_deploy_app_script(... OUTPUT_SCRIPT ...)`, whose
`OUTPUT_SCRIPT` signature only exists from Qt 6.5 onward. This fails to
even *configure* on any machine with Qt 6.4.x (a very common LTS/distro
version, e.g. Ubuntu 24.04's default).

**Fix:** lowered the requirement to `Qt6 6.4` and guarded the deploy-
script call behind `if(QT_VERSION VERSION_GREATER_EQUAL 6.5)`.

## 6. Missing feature — hours ↔ minutes conversion, AND rate ↔ mean input mode

Per the assignment brief, the calculator now includes two independent
input controls in the Input Parameters panel:

- **Time Unit**: Hours / Minutes
- **Input As**: Rate (λ, μ) / Mean Time (1/λ, 1/μ)

These are genuinely different conversions and are kept separate:

| Conversion | What it changes | Rule |
|---|---|---|
| **Time Unit** (Hours↔Minutes) on a **rate** field | the time reference of a rate | divide/multiply by 60 |
| **Time Unit** (Hours↔Minutes) on a **mean time** field | the time reference of a mean | multiply/divide by 60 (opposite direction) |
| **Time Unit** on a **variance** field | time² reference | multiply/divide by 3600 |
| **Input As** (Rate↔Mean) | the *form* of the same physical quantity, same unit | reciprocal: mean = 1/rate |

Implemented in `TimeUnitConverter.h/.cpp`:
- `convertRate()` / `convertMean()` / `convertVariance()` — the Hours↔Minutes
  conversions (rate-wise, mean-wise, variance-wise respectively).
- `toRate()` / `toMean()` — the **separate** reciprocal conversion used by
  the Rate↔Mean input-mode toggle. This is not a unit conversion, so it
  is deliberately a different function.

**UI behavior:**
- Switching **Time Unit** converts whatever is currently in the arrival/
  service boxes using rate-wise or mean-wise rules, *depending on which
  Input Mode is currently active* (this is the subtlety a naive
  implementation gets wrong — the same combo box conversion cannot use
  a single fixed rule).
- Switching **Input As** inverts the two values (mean = 1/rate) and
  swaps the field captions/suffixes between "λ Arrival Rate / μ Service
  Rate" and "Mean Inter-arrival Time / Mean Service Time".
- Whichever mode is active, `MainWindow::resolveRates()` is called
  before every calculation to normalize both fields back into rates
  (λ, μ) for the model classes — inverting mean values with
  `TimeUnitConverter::toRate()` if needed. This was the one gap found
  during testing: the mode-switching UI logic existed, but
  `onCalculateClicked()` was still reading the spin boxes directly and
  feeding them to the models as if they were always rates, so
  "Mean Time" mode silently produced wrong answers. Fixed by routing
  everything through `resolveRates()`, with its own mode-aware
  validation messages (e.g. "Mean Inter-arrival Time must be greater
  than zero.").
- The Wq and Ws result cards show their value in the currently selected
  time unit, with a small caption underneath showing the same value
  converted (mean-wise) into the other time unit.

**Verified:** entering λ=4/hr, μ=5/hr in Rate mode, and entering the
equivalent mean times (0.25 hr, 0.2 hr) in Mean mode, produce identical
Lq/Wq results (Lq=3.2, Wq=0.8 for both) — confirming the reciprocal
conversion round-trips correctly through the calculation engine.

## 7. Minor cleanups

- `MM1.cpp`: removed a duplicate `result.setUtilization(rho)` call and
  switched the stability check to the shared
  `InputValidator::isSingleServerStable()` helper for consistency.
- `GG1.cpp`: switched its inline `utilization >= 1` stability check to
  the same shared helper.

## Verification performed

- Installed Qt6 (6.4.2) + CMake and did a clean `cmake && make` build
  of both the original project (to confirm a working baseline) and the
  fixed project (zero errors, zero warnings).
- Ran the built binary offscreen (`QT_QPA_PLATFORM=offscreen`) to
  confirm it starts and stays alive without crashing.
- Wrote small standalone test programs to check that:
  - `MG1` reduces exactly to `MM1` when service-time variance = 1/μ²
    (Poisson/exponential case).
  - `GG1` reduces exactly to `MM1` when Ca² = Cs² = 1.
  - `MMS` reproduces standard textbook M/M/S results (λ=10, μ=4, s=3 →
    ρ=0.8333, Lq≈3.511).
  - `MGS` and `GGS` closely approximate the exact `MMS` result when
    given exponential (Ca²=Cs²=1) variances, with only the small
    error inherent to the Sakasegawa approximation itself.
  - `TimeUnitConverter` round-trips correctly (120/hr ↔ 2/min,
    0.5 hr ↔ 30 min, 1 hr² ↔ 3600 min²).
