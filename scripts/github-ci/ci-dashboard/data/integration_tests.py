import json
import sys
from pathlib import Path

sys.path.append(str(Path(__file__).resolve().parents[4]))
from parsers.junit_xml import JUnitXMLParser

def load_integration_test_data(json_file: str):
    with open(json_file) as f:
        integration_test_data = json.load(f)
    return integration_test_data