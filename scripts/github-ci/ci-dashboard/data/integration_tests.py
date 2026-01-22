import json
import sys
from pathlib import Path

sys.path.append(str(Path(__file__).resolve().parents[4]))
from parsers.junit_xml import JUnitXMLParser

def load_integration_test_data(core_json_file: str, extended_json_file: str):
    core_summary = IntegrationTestSummary.from_json_file(core_json_file)
    extended_summary = IntegrationTestSummary.from_json_file(extended_json_file)
    combined_summary = IntegrationTestSummary.combined(core_summary, extended_summary)
    return combined_summary