#pragma once

namespace GTS {

	struct FindActorData final {
		/// Actors that have been done recently;
		absl::flat_hash_set<FormID> previousActors;
	};

	[[nodiscard]] const std::vector<Actor*>& find_actors();
	[[nodiscard]] std::vector<Actor*> FindSomeActors(std::string_view tag, uint32_t howMany);
	[[nodiscard]] const std::vector<Actor*>& FindTeammates();
	[[nodiscard]] const std::vector<Actor*>& FindFemaleTeammates();

}
