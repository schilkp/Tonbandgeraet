import argparse
import re
import subprocess
from dataclasses import dataclass
from itertools import repeat
from multiprocessing import Pool
from os import listdir
from os.path import basename, isdir, isfile, join
from typing import List, Literal, Optional, Union


class Color:
    OK = '\033[92m'
    WARN = '\033[93m'
    ERR = '\033[91m'
    INFO = '\033[96m'
    END = '\33[0m'


@dataclass
class TestResult:
    result: Union[Literal['PASS'], Literal['FAIL'], Literal['IGNORE']]
    result_line_raw: str
    stdout: str

    def result_line(self) -> str:
        # Unity test results are of the form:
        #   SOURCE_NAME:LINE_NO:TEST_NAME:RESULT
        #
        # We strip the path from SOURCE_NAME to make the output a bit less spammy.
        parts = self.result_line_raw.split(":")
        parts[0] = basename(parts[0])
        return ":".join(parts)

    def name(self) -> str:
        parts = self.result_line_raw.split(":")
        return parts[2]

    def generate_report(self) -> List[str]:
        out = []
        out.append(f"  {self.name()}:")
        out.append(f"    result: {self.result}")
        out.append(f"    result_line: \"{self.result_line_raw}\"")
        if self.stdout:
            out.append('    stdout: |')
            for line in self.stdout:
                out.append('        ' + line)
        return out


@dataclass
class OkSuiteResult:
    return_val: int
    reported_test_count: int
    reported_fail_count: int
    reported_ignore_count: int
    stderr: List[str]

    def generate_report(self) -> List[str]:
        out = []
        if self.reported_fail_count > 0:
            out.append('result: FAIL')
        else:
            out.append('result: OK')
        if self.stderr:
            out.append('stderr: |')
            for line in self.stderr:
                out.append('  ' + line)
        return out


@dataclass
class CrashedSuiteResult:
    return_val: Optional[int]
    msg: Optional[str]
    stderr: List[str]

    def generate_report(self) -> List[str]:
        out = []
        out.append('result: CRASHED')
        if self.msg:
            out.append('msg: {self.msg}')
        if self.stderr:
            out.append('stderr: |')
            for line in self.stderr:
                out.append('  ' + line)
        return out


@dataclass
class SuiteResult:
    path: str
    result: OkSuiteResult | CrashedSuiteResult
    test_results: List[TestResult]

    def test_count(self) -> int:
        return len(self.test_results)

    def fail_count(self) -> int:
        return len([t for t in self.test_results if t.result == 'FAIL'])

    def ignore_count(self) -> int:
        return len([t for t in self.test_results if t.result == 'IGNORE'])

    def pass_count(self) -> int:
        return len([t for t in self.test_results if t.result == 'PASS'])

    def name(self) -> str:
        return basename(self.path)

    def generate_report(self) -> List[str]:
        out = []
        out.append(f'---')
        out.append(f'path: {self.path}')
        out.extend(self.result.generate_report())
        out.append(f'test_count: {self.test_count()}')
        out.append(f'pass_count: {self.pass_count()}')
        out.append(f'fail_count: {self.fail_count()}')
        out.append(f'ignore_count: {self.ignore_count()}')
        if len(self.test_results) > 0:
            out.append(f'tests:')
            for test in self.test_results:
                out.extend(test.generate_report())
        return out


def run_test_suite(path: str, timeout_ms: Optional[float]) -> SuiteResult:
    result = execute_suite_executeable(path, timeout_ms)

    with open(path + "_report.yml", 'w') as f:
        for line in result.generate_report():
            f.write(line + "\n")

    return result


def execute_suite_executeable(path: str, timeout_ms: Optional[float]) -> SuiteResult:
    try:
        test_output = subprocess.run(
            path, capture_output=True, timeout=timeout_ms)

        stdout = test_output.stdout.decode('utf-8').strip().splitlines()
        stderr = test_output.stderr.decode('utf-8').strip().splitlines()

        # Write stdout + stderr files:
        with open(path + "_stdout.txt", 'w') as f:
            f.writelines(map(lambda x: x+'\n', stdout))
        if len(stderr) > 0:
            with open(path + "_stderr.txt", 'w') as f:
                f.writelines(map(lambda x: x+'\n', stderr))

        suite_result = parse_suite_footer(
            test_output.returncode, stdout, stderr)
        test_results = parse_suite_output(stdout)

        if isinstance(suite_result, CrashedSuiteResult):
            return SuiteResult(path=path, result=suite_result, test_results=test_results)
        else:
            result = SuiteResult(
                path=path, result=suite_result, test_results=test_results)

            # If test suite did not crash, check that return value and reported counts check out:
            check = [(suite_result.return_val, result.fail_count()),
                     (suite_result.reported_test_count, result.test_count()),
                     (suite_result.reported_fail_count, result.fail_count()),
                     (suite_result.reported_ignore_count, result.ignore_count())]

            for a, b in check:
                if a != b:
                    msg = "Test result count mismatch."
                    result = CrashedSuiteResult(
                        test_output.returncode, msg=msg,  stderr=stderr)
                    return SuiteResult(path=path, result=result, test_results=test_results)

            return result

    except subprocess.TimeoutExpired:
        return SuiteResult(path=path, result=CrashedSuiteResult(return_val=None, msg="Timeout", stderr=[]), test_results=[])


def parse_suite_output(stdout: List[str]) -> List[TestResult]:
    # Remove footer:
    if len(stdout) < 3:
        return []
    stdout = stdout[:-3]

    results = []  # type: List[TestResult]
    t_stdout = []  # type: List[str]

    for line in stdout:
        if ':FAIL' in line:
            results.append(TestResult('FAIL', line, t_stdout))  # type: ignore
            t_stdout = []
        elif ':PASS' in line:
            results.append(TestResult('PASS', line, t_stdout))  # type: ignore
            t_stdout = []
        elif ':IGNORE' in line:
            results.append(TestResult('IGNORE', line, t_stdout)) # type: ignore
            t_stdout = []
        else:
            t_stdout.append(line)

    return results


FOOTER_RE = re.compile(r"^-----------------------\n(\d+) Tests (\d+) Failures (\d+) Ignored\n(OK|FAILED|FAIL)$")


def parse_suite_footer(returncode: int, stdout: List[str], stderr: List[str]) -> CrashedSuiteResult | OkSuiteResult:

    if len(stdout) < 3:
        return CrashedSuiteResult(return_val=returncode, stderr=stderr, msg="Output too short to contain footer")

    footer = "\n".join(
        [stdout[-3].strip(), stdout[-2].strip(), stdout[-1].strip()])

    match = FOOTER_RE.match(footer)

    if match:
        groups = match.groups()
        try:
            reported_test_count = int(groups[0])
            reported_fail_count = int(groups[1])
            reported_ignored_count = int(groups[2])
        except ValueError:
            return CrashedSuiteResult(return_val=returncode, stderr=stderr, msg="Malformed footer")

        return OkSuiteResult(returncode, reported_test_count, reported_fail_count, reported_ignored_count, stderr)
    else:
        return CrashedSuiteResult(return_val=returncode, stderr=stderr, msg="Malformed footer")


def main():

    parser = argparse.ArgumentParser(prog='Unity Test Runner')
    parser.add_argument('-j', '--jobs', type=int,
                        help="allow multiple jobs to run at once.", default=1)
    parser.add_argument('-t', '--timeout', type=float,
                        help="timeout for test suite execution in milliseconds.", required=False)
    parser.add_argument('-p', '--print-fail-stdout', action="store_true",
                        help='print stdout of failed tests.', default=False)
    parser.add_argument('test_suite', type=str, nargs='+',
                        help='test suite executeable or directory')
    args = parser.parse_args()

    # Discover test suites:
    files = []
    for suite in args.test_suite:
        if isdir(suite):
            for file in listdir(suite):
                file = join(suite, file)
                if isfile(file):
                    files.append(file)
        else:
            files.append(suite)

    test_suites = []
    for file in files:
        if file.endswith('_report.yml'):
            continue
        if file.endswith('_stdout.txt'):
            continue
        if file.endswith('_stderr.txt'):
            continue
        test_suites.append(file)

    # Print message and exit if there where no tests passed:
    if len(test_suites) == 0:
        print("No test suites found.")
        exit(0)

    print()
    print("Running tests...")
    # Run tests:
    timeout = args.timeout  # type: float | None
    if args.jobs <= 1:
        results = [run_test_suite(suite, timeout)
                   for suite in test_suites]
    else:
        with Pool(args.jobs) as p:
            results = p.starmap(run_test_suite, zip(
                test_suites, repeat(timeout)))

    for suite in sorted(results, key=(lambda s: s.name())):
        if isinstance(suite.result, CrashedSuiteResult):
            print(Color.ERR + suite.name() + ": Crashed!", end="")
            if suite.result.msg:
                print(f" ({suite.result.msg})", end="")
            print(Color.END)
        else:
            print(Color.INFO + suite.name() + ":" + Color.END)
            for test in suite.test_results:
                match test.result:
                    case 'PASS':
                        print(Color.OK, end="")
                    case 'IGNORE':
                        print(Color.WARN, end="")
                    case 'FAIL':
                        print(Color.ERR, end="")
                print("  " + test.result_line() + Color.END)

    crashed_suites = [s for s in results if isinstance(
        s.result, CrashedSuiteResult)]
    ok_suites = [s for s in results if isinstance(s.result, OkSuiteResult)]

    crashes = len(crashed_suites)
    passes = sum([s.pass_count() for s in ok_suites])
    fails = sum([s.fail_count() for s in ok_suites])
    ignores = sum([s.ignore_count() for s in ok_suites])
    tests = passes + fails + ignores

    if fails > 0:
        print()
        print("========== Failed tests ===========")
        for suite in ok_suites:
            if suite.fail_count == 0:
                continue
            for test in suite.test_results:
                if test.result == 'FAIL':
                    if args.print_fail_stdout:
                        print(Color.ERR + test.result_line() + ":" + Color.END)
                        for line in test.stdout:
                            print(Color.ERR + "  " + line + Color.END)
                    else:
                        print(Color.ERR + test.result_line() + Color.END)

    print()
    print("============= Summary =============")

    if crashes != 0:
        print(Color.ERR, end='')
        print("Warning! %i test suite(s) crashed. Not all tests were performed!" % crashes)
        print(Color.END, end='')

    print("Ran %i tests." % tests)

    print(Color.ERR, end='')
    print("Failed: %i" % fails)
    print(Color.END + Color.OK, end='')
    print("Passed: %i" % passes)
    print(Color.END + Color.WARN, end='')
    print("Ignore: %i" % ignores)
    print(Color.END, end='')

    if (tests - ignores) != 0:
        print("Success rate (without ignored tests): %2.2f%%" %
              (float(passes) / (tests - ignores) * 100))

    print()
    print("===================================")
    if crashes == 0 and fails == 0:
        print(Color.OK, end='')
        print("All good! :)")
        print(Color.END)
        print()
        print()
        exit(0)
    else:
        print(Color.WARN, end='')
        print("There is some work left to do...")
        print(Color.END, end='')
        print()
        print()
        exit(1)

if __name__ == '__main__':
    main()
