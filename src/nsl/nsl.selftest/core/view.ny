// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


class Dummy {
	view (ref filter): ref -> new class {
		func cursor: ref {
			return new class {
				func findFirst: bool -> true;

				func next: bool {
					m_index += 1u;
					return m_index < 4u;
				}

				func get: ref -> m_index;

				var m_index = 0u;
			};
		}
	}
}

unittest std.core.view.simple {
	var sum = 0u;
	for i in (new Dummy) do
		sum += i;
	assert(sum == 6u);
}


class WithCapture {
	view (ref filter): ref {
		ref predicate = filter;
		return new class {
			func cursor: ref {
				ref filter = predicate;
				return new class {
					func findFirst: bool -> true;

					func next: bool {
						m_index += 1u;
						return m_index < 4u;
					}

					func get: ref -> m_index;

					var m_index = 0u;
				};
			}
		};
	}
}

unittest std.core.view.capture {
	var sum = 0u;
	for i in (new WithCapture) do
		sum += i;
	assert(sum == 6u);
}


class WithCaptureAndUse {
	view (ref filter): ref {
		ref predicate = filter;
		return new class {
			func cursor: ref {
				ref accept = predicate;
				return new class {
					func findFirst: bool -> accept(m_index) or next();

					func next: bool {
						do {
							m_index += 1u;
							if m_index >= 4u then
								return false;
						}
						while not accept(m_index);
						return true;
					}

					func get: ref -> m_index;

					var m_index = 0u;
				};
			}
		};
	}
}

unittest std.core.view.captureAndUse {
	var sum = 0u;
	for i in (new WithCaptureAndUse) do {
		console << i << '\n';
		sum += i;
	}
	assert(sum == 6u);
}
