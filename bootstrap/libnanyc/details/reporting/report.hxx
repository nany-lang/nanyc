#pragma once
#include "report.h"


namespace ny {
namespace Logs {


inline Report::Report(Message& message)
	: message(message) {
}


inline Report::Report(Report& report)
	: message(report.message) {
}


inline Report::Report(Report&& report)
	: message(report.message) {
}


template<class T>
inline Report& Report::operator << (const T& value) {
	message.message.append(value);
	return *this;
}


inline YString& Report::text() {
	return message.message;
}


inline const YString& Report::text() const {
	return message.message;
}


inline bool Report::hasErrors() const {
	return message.hasErrors;
}


inline Message& Report::data() {
	return message;
}


inline Message::Origin& Report::origins() {
	return message.origins;
}


template<class T>
inline EmptyReport& EmptyReport::operator << (const T&) {
	return *this;
}


inline bool EmptyReport::hasErrors() const {
	return false;
}


inline void Report::appendEntry(const Message::Ptr& entry) {
	message.appendEntry(entry);
}


inline Report::operator bool () const {
	return message.level >= Level::warning;
}


} // namespace Logs
} // namespace ny
