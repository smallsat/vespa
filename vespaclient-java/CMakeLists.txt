# Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
install_fat_java_artifact(vespaclient-java)

vespa_install_script(src/main/sh/vespa-document-statistics.sh vespa-document-statistics bin)
vespa_install_script(src/main/sh/vespa-stat.sh vespa-stat bin)
vespa_install_script(src/main/sh/vespa-query-profile-dump-tool.sh vespa-query-profile-dump-tool bin)
vespa_install_script(src/main/sh/vespa-summary-benchmark.sh vespa-summary-benchmark bin)
vespa_install_script(src/main/sh/vespa-destination.sh vespa-destination bin)
vespa_install_script(src/main/sh/vespa-feeder.sh vespa-feeder bin)
vespa_install_script(src/main/sh/vespa-get.sh vespa-get bin)
vespa_install_script(src/main/sh/vespa-visit.sh vespa-visit bin)
vespa_install_script(src/main/sh/vespa-visit-target.sh vespa-visit-target bin)
