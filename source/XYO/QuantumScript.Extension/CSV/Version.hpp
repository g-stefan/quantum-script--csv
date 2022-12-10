// Quantum Script Extension CSV
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_CSV_VERSION_HPP
#define XYO_QUANTUMSCRIPT_EXTENSION_CSV_VERSION_HPP

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_CSV_DEPENDENCY_HPP
#	include <XYO/QuantumScript.Extension/CSV/Dependency.hpp>
#endif

namespace XYO::QuantumScript::Extension::CSV::Version {

	XYO_QUANTUMSCRIPT_EXTENSION_CSV_EXPORT const char *version();
	XYO_QUANTUMSCRIPT_EXTENSION_CSV_EXPORT const char *build();
	XYO_QUANTUMSCRIPT_EXTENSION_CSV_EXPORT const char *versionWithBuild();
	XYO_QUANTUMSCRIPT_EXTENSION_CSV_EXPORT const char *datetime();

};

#endif