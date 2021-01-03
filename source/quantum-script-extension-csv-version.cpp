//
// Quantum Script Extension CSV
//
// Copyright (c) 2020-2021 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#include "quantum-script-extension-csv-version.hpp"

namespace Quantum {
	namespace Script {
		namespace Extension {
			namespace CSV {
				namespace Version {

					static const char *version_ = "1.3.0";
					static const char *build_ = "4";
					static const char *versionWithBuild_ = "1.3.0.4";
					static const char *datetime_ = "2021-01-03 02:30:52";

					const char *version() {
						return version_;
					};
					const char *build() {
						return build_;
					};
					const char *versionWithBuild() {
						return versionWithBuild_;
					};
					const char *datetime() {
						return datetime_;
					};

				};
			};
		};
	};
};



