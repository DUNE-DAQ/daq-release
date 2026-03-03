#!/usr/bin/env python3

import os
import subprocess

from pathlib import Path

def run_command(
    cmd,
    cwd=None,
    check=True,
    context=None,
    continue_on_error=False
):
    """
    Run a shell command and return a dictionary with its results.

    Parameters:
        cmd: list or str - the command to run (list preferred)
        cwd: str - directory to run the command in
        check: bool - raise RuntimeError if the command fails
        context: str - optional extra message for errors
        continue_on_error: bool - if True, don't raise, just return dict

    Returns:
        dict with keys: success, stdout, stderr, exit_code, command, context
    """

    result_dict = {
        "success": False,
        "stdout": "",
        "stderr": "",
        "exit_code": None,
        "command": cmd,
        "context": context,
    }

    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            shell=True,
            check=check,
            cwd=cwd
        )
        result_dict.update({
            "success": True,
            "stdout": result.stdout.strip(),
            "stderr": result.stderr.strip(),
            "exit_code": result.returncode
        })
        if result_dict["stdout"]:
            print(result_dict["stdout"])
    except subprocess.CalledProcessError as e:
        result_dict.update({
            "stdout": e.stdout.strip() if e.stdout else "",
            "stderr": e.stderr.strip() if e.stderr else "",
            "exit_code": e.returncode
        })

        message = (
            f"Command failed: {result_dict['command']}\n"
            f"Exit code: {result_dict['exit_code']}\n"
            f"Stdout:\n{result_dict['stdout']}\n"
            f"Stderr:\n{result_dict['stderr']}"
        )
        if context:
            message = f"{context}\n\n{message}"

        if continue_on_error:
            print(message)
            print("Continuing despite error...")
        else:
            raise RuntimeError(message) from e

    return result_dict

if __name__ == "__main__":
    pass
    