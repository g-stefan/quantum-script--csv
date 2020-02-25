//
// Quantum Script Extension CSV
//
// Copyright (c) 2020 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quantum-script-extension-csv-license.hpp"
#include "quantum-script-extension-csv.hpp"
#ifndef QUANTUM_SCRIPT_EXTENSION_CSV_NO_VERSION
#include "quantum-script-extension-csv-version.hpp"
#endif

#include "quantum-script-variablestring.hpp"

//#define QUANTUM_SCRIPT_VM_DEBUG_RUNTIME

namespace Quantum {
	namespace Script {
		namespace Extension {
			namespace CSV {

				using namespace XYO;
				using namespace Quantum::Script;

				static TPointer<Variable> decode(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- csv-decode\n");
#endif
					String in = (arguments->index(0))->toString();
					Variable *out = VariableArray::newVariable();
					String out_;
					size_t k;
					size_t ln;

					out_ = "";
					ln = 0;
					for(k = 0; k < in.length(); ++k) {
						if(in[k] == ',') {
							(out->operatorIndex(ln))=VariableString::newVariable(out_);
							ln++;
							out_ = "";
							continue;
						};
						if(in[k] == '\"') {
							++k;
							for(; k < in.length(); ++k) {
								if(in[k] == '\"') {
									if(k + 1 < in.length()) {
										if(in[k + 1] == '\"') {
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
					(out->operatorIndex(ln))=VariableString::newVariable(out_);

					return out;
				};

				static TPointer<Variable> encode(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- csv-encode\n");
#endif

					Variable *in = (arguments->index(0)).value();
					String out;

					String out_;
					bool useComa;
					bool useEscape;

					if(!VariableArray::isVariableArray(in)) {
						throw(Error("invalid parameter"));
					};

					size_t k;
					size_t sz;

					useComa = false;
					for(k = 0; k < ((VariableArray *)in)->value->length(); ++k) {
						if(useComa) {
							out << ',';
						};
						useEscape = false;
						out_ = (in->operatorIndex(k))->toString();
						if(String::indexOf(out_, " ", 0, sz)) {
							useEscape = true;
						};
						if(String::indexOf(out_, "\t", 0, sz)) {
							useEscape = true;
						};
						if(String::indexOf(out_, ",", 0, sz)) {
							useEscape = true;
						};
						if(String::indexOf(out_, "\"", 0, sz)) {
							useEscape = true;
							out_ = String::replace(out_, "\"", "\"\"");
						};
						if(useEscape) {
							out << '"';
						};
						out << out_;
						if(useEscape) {
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
					info << License::shortContent();

					executive->setExtensionName(extensionId, "CSV");
					executive->setExtensionInfo(extensionId, info);
#ifndef QUANTUM_SCRIPT_EXTENSION_CSV_NO_VERSION
					executive->setExtensionVersion(extensionId, Extension::CSV::Version::versionWithBuild());
#endif
					executive->setExtensionPublic(extensionId, true);

					executive->compileStringX("var CSV={};");
					executive->setFunction2("CSV.decode(str)", decode);
					executive->setFunction2("CSV.encode(value)", encode);
				};

			};
		};
	};
};

#ifdef XYO_COMPILE_DYNAMIC_LIBRARY
extern "C" QUANTUM_SCRIPT_EXTENSION_CSV_EXPORT void quantumScriptExtension(Quantum::Script::Executive *executive, void *extensionId) {
	Quantum::Script::Extension::CSV::initExecutive(executive, extensionId);
};
#endif

