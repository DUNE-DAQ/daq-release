from pathlib import Path

class Page:
    def __init__(self, template_name, context, output_file=None):
        self.template_name = template_name
        self.context = context
        self.output_file = output_file

    def render(self, renderer):
        output_file = Path(self.output_file) or Path(self.template_name)
        renderer.render_page(self.template_name, self.context, output_file)

class IndexPage(Page):
    def __init__(self, index_context):
        super().__init__(
            template_name="index_template.html",
            context=index_context,
            output_file="index.html"
        )

class RepoPage(Page):
    def __init__(self, repo_name, context):
        super().__init__(
            template_name="repo_template.html",
            context=context,
            output_file=f"repos/{repo_name}.html"
        )

class UnitTestPage(Page):
    def __init__(self, unit_test_summary):
        super().__init__(
            template_name="unit_test_template.html",
            context=unit_test_summary,
            output_file="unit_test_summary.html"
        )

class IntegrationTestPage(Page):
    def __init__(self, integration_test_summary):
        super().__init__(
            template_name="integration_test_template.html",
            context=integration_test_summary,
            output_file="integtest_summary.html"
        )