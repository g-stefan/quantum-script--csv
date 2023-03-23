// Quantum Script Extension CSV
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/QuantumScript.Extension/CSV/Library.hpp>
#include <XYO/QuantumScript.Extension/CSV/Copyright.hpp>
#include <XYO/QuantumScript.Extension/CSV/License.hpp>
#include <XYO/QuantumScript.Extension/CSV/Version.hpp>

namespace XYO::QuantumScript::Extension::CSV {

	static TPointer<Variable> decode(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- csv-decode\n");
#endif
		String in = (arguments->index(0))->toString();
		Variable *out = VariableArray::newVariable();
		String out_;
		size_t k;
		size_t ln;

		out_ = "";
		ln = 0;
		for (k = 0; k < in.length(); ++k) {
			if (in[k] == ',') {
				out->setPropertyByIndex(ln, VariableString::newVariable(out_));
				ln++;
				out_ = "";
				continue;
			};
			if (in[k] == '\"') {
				++k;
				for (; k < in.length(); ++k) {
					if (in[k] == '\"') {
						if (k + 1 < in.length()) {
							if (in[k + 1] == '\"') {
								out_ << '\"';
								++k;
								continue;
							};
						};
						break;
					};
					out_ << in[k];
				};
				continue;
			};
			out_ << in[k];
		};
		out->setPropertyByIndex(ln, VariableString::newVariable(out_));

		return out;
	};

	static TPointer<Variable> encode(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- csv-encode\n");
#endif

		Variable *in = (arguments->index(0)).value();
		String out;

		String out_;
		bool useComa;
		bool useEscape;

		if (!TIsType<VariableArray>(in)) {
			throw(Error("invalid parameter"));
		};

		size_t k;
		size_t sz;

		useComa = false;
		for (k = 0; k < ((VariableArray *)in)->value->length(); ++k) {
			if (useComa) {
				out << ',';
			};
			useEscape = false;
			out_ = (in->getPropertyByIndex(k))->toString();
			if (StringX::indexOf(out_, " ", 0, sz)) {
				useEscape = true;
			};
			if (StringX::indexOf(out_, "\t", 0, sz)) {
				useEscape = true;
			};
			if (StringX::indexOf(out_, ",", 0, sz)) {
				useEscape = true;
			};
			if (StringX::indexOf(out_, "\"", 0, sz)) {
				useEscape = true;
				out_ = StringX::replace(out_, "\"", "\"\"");
			};
			if (useEscape) {
				out << '"';
			};
			out << out_;
			if (useEscape) {
				out << '"';
			};
			useComa = true;
		};

		return VariableString::newVariable(out);
	};

	void registerInternalExtension(Executive *executive) {
		executive->registerInternalExtension("CSV", initExecutive);
	};

	void initExecutive(Executive *executive, void *extensionId) {

		String info = "CSV\r\n";
		info << License::shortLicense().c_str();

		executive->setExtensionName(extensionId, "CSV");
		executive->setExtensionInfo(extensionId, info);
		executive->setExtensionVersion(extensionId, Extension::CSV::Version::versionWithBuild());
		executive->setExtensionPublic(extensionId, true);

		executive->compileStringX("var CSV={};");
		executive->setFunction2("CSV.decode(str)", decode);
		executive->setFunction2("CSV.encode(value)", encode);
	};

};

#ifdef XYO_COMPILE_DYNAMIC_LIBRARY
extern "C" XYO_QUANTUMSCRIPT_EXTENSION_CSV_EXPORT void quantumScriptExtension(XYO::QuantumScript::Executive *executive, void *extensionId) {
	XYO::QuantumScript::Extension::CSV::initExecutive(executive, extensionId);
};
#endif
