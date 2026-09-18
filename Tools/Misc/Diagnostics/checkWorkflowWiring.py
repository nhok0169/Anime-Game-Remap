"""
Checks the reusable-workflow wiring under .github/workflows the way GitHub would only discover at run
time, and prints the status-check names branch protection has to require.

    python checkWorkflowWiring.py            exit 1 on any problem

Checks:
  * every input a caller passes is declared by the workflow it calls, and every required one is passed
  * every 'needs.X.outputs.Y' names a job in 'needs' and an output that job's workflow declares
  * every 'needs:' names a job that exists
  * nothing calls a tester without the build artifact -- it would fail at 'import FixRaidenBoss2', since
    the testers import the API from the source tree and no compiled module is tracked. This is the check
    that caught a publish flow calling the testers with no build, which reading the files had missed
  * reusable nesting stays within GitHub's limit of 4

Prints, per workflow that is not only reusable, the CHECK NAMES its jobs report. A required status check
is matched on that name -- the chain of caller job names down to the job that runs -- so renaming a
workflow's job, or a job inside a workflow it calls, silently strands branch protection: the pull
request waits forever on 'Expected -- Waiting for status to be reported'. A '${{ ... }}' left in a name
is filled in per run (a matrix value, an input); GitHub shows the resolved text.

Needs PyYAML. Reads only; changes nothing.
"""

import pathlib
import re
import sys

import yaml

Root = pathlib.Path(__file__).resolve().parents[3]
Workflows = Root / ".github" / "workflows"
Testers = {"unit-test-workflow.yml", "integration-test-workflow.yml"}


def triggers(doc):
    # 'on' is read by YAML 1.1 as the boolean True
    return doc.get("on", doc.get(True)) or {}


def callTarget(uses):
    if (isinstance(uses, str) and uses.startswith("./")):
        return pathlib.Path(uses[2:]).name
    return None


def needsOf(job):
    needs = job.get("needs") or []
    return [needs] if isinstance(needs, str) else needs


def checkNames(docs, name, prefix = ()):
    """
    The name each leaf job reports as a status check, by walking reusable calls.
    """

    result = []
    for jobId, job in ((docs.get(name) or {}).get("jobs") or {}).items():
        label = str(job.get("name") or jobId)
        target = callTarget(job.get("uses"))
        if (target and target in docs):
            result += checkNames(docs, target, prefix + (label,))
        else:
            result.append(" / ".join(prefix + (label,)))
    return result


def depth(docs, name, seen = ()):
    if (name in seen):
        return 99
    best = 1
    for job in ((docs.get(name) or {}).get("jobs") or {}).values():
        target = callTarget(job.get("uses"))
        if (target and target in docs):
            best = max(best, 1 + depth(docs, target, seen + (name,)))
    return best


if (__name__ == "__main__"):
    docs = {path.name: yaml.safe_load(path.read_text(encoding = "utf-8")) for path in sorted(Workflows.glob("*.yml"))}
    problems = []

    for name, doc in docs.items():
        jobs = doc.get("jobs") or {}
        for jobId, job in jobs.items():
            needs = needsOf(job)
            for n in needs:
                if (n not in jobs):
                    problems.append(f"{name}: job '{jobId}' needs '{n}', which is not a job here")

            target = callTarget(job.get("uses"))
            if (target is None):
                continue
            if (target not in docs):
                problems.append(f"{name}: job '{jobId}' calls '{target}', which does not exist")
                continue

            declared = (triggers(docs[target]).get("workflow_call") or {}).get("inputs") or {}
            passed = job.get("with") or {}
            for key in passed:
                if (key not in declared):
                    problems.append(f"{name}: job '{jobId}' passes '{key}' to {target}, which does not declare it")
            for key, spec in declared.items():
                if ((spec or {}).get("required") and key not in passed):
                    problems.append(f"{name}: job '{jobId}' omits required input '{key}' of {target}")

            if (target in Testers and not passed.get("artifact")):
                problems.append(f"{name}: job '{jobId}' calls {target} with NO build artifact -- it will fail at import")

            for depJob, outName in re.findall(r"needs\.([A-Za-z0-9_-]+)\.outputs\.([A-Za-z0-9_-]+)", yaml.dump(passed)):
                if (depJob not in needs):
                    problems.append(f"{name}: job '{jobId}' reads needs.{depJob} but does not list it in needs")
                    continue
                depTarget = callTarget((jobs.get(depJob) or {}).get("uses"))
                if (depTarget and depTarget in docs):
                    outputs = (triggers(docs[depTarget]).get("workflow_call") or {}).get("outputs") or {}
                    if (outName not in outputs):
                        problems.append(f"{name}: job '{jobId}' reads needs.{depJob}.outputs.{outName}, which {depTarget} does not declare")

    # every local action a step uses must exist
    for name, doc in docs.items():
        for jobId, job in (doc.get("jobs") or {}).items():
            for step in job.get("steps") or []:
                uses = step.get("uses")
                if (isinstance(uses, str) and uses.startswith("./") and not (Root / uses[2:] / "action.yml").exists()):
                    problems.append(f"{name}: job '{jobId}' uses '{uses}', which has no action.yml")

    # the wheels' z3 is warmed on master by warm-caches.yml and restored by python-publish.yml: the cache
    #   key includes the runner and the macOS target, so a runner or target in one and not the other is a
    #   release that silently builds z3 from cold on that runner
    def wheelExterns(fileName, jobId):
        job = ((docs.get(fileName) or {}).get("jobs") or {}).get(jobId) or {}
        runners = ((job.get("strategy") or {}).get("matrix") or {}).get("os") or []
        targets = {str((s.get("with") or {}).get("macos-deployment-target"))
                   for s in job.get("steps") or [] if str(s.get("uses", "")).endswith("wheel-externs")}
        env = {}
        for s in job.get("steps") or []:
            env.update(s.get("env") or {})
        return sorted(runners), targets, env

    if ("python-publish.yml" in docs and "warm-caches.yml" in docs):
        pubRunners, pubTargets, pubEnv = wheelExterns("python-publish.yml", "build-wheels")
        warmRunners, warmTargets, _ = wheelExterns("warm-caches.yml", "wheel-externs")
        if (pubRunners != warmRunners):
            problems.append(f"wheel runners differ: python-publish.yml {pubRunners} vs warm-caches.yml {warmRunners}")
        if (pubTargets != warmTargets or len(pubTargets) != 1):
            problems.append(f"macOS deployment target differs: python-publish.yml {pubTargets} vs warm-caches.yml {warmTargets}")
        macEnv = str(pubEnv.get("CIBW_ENVIRONMENT_MACOS", ""))
        for target in pubTargets:
            if (f"MACOSX_DEPLOYMENT_TARGET={target}" not in macEnv):
                problems.append(f"python-publish.yml builds z3 for macOS {target} but its wheels with '{macEnv}'")

    entryPoints = [n for n in docs if "workflow_call" not in triggers(docs[n]) or len(triggers(docs[n])) > 1]
    for name in entryPoints:
        d = depth(docs, name)
        if (d > 4):
            problems.append(f"{name}: nests {d} levels of reusable workflows (GitHub's limit is 4)")

    print(f"checked {len(docs)} workflows under {Workflows.relative_to(Root)}")
    print("\nwiring is consistent" if not problems else f"\n{len(problems)} problem(s):")
    for p in problems:
        print("  " + p)

    print("\ncheck names each workflow reports (what branch protection must require):")
    for name in entryPoints:
        on = triggers(docs[name])
        events = ", ".join(str(k) for k in (on.keys() if isinstance(on, dict) else [on]))
        print(f"\n  {name}   [{docs[name].get('name', '')}]   on: {events}")
        for check in checkNames(docs, name):
            print(f"      {check}")

    sys.exit(1 if problems else 0)
