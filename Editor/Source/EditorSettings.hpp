#pragma once

// ---------------------------------------------------------------------------
// Editor settings.
//
// The state behind View > Show Grid, kept in one struct so that M7 can write the
// whole thing to editor.cfg without anything having to be re-plumbed. Only what
// something actually reads lives here: a settings field nobody uses is a field
// that quietly rots.
// ---------------------------------------------------------------------------
struct EditorSettings
{
	struct Grid
	{
		bool visible{ true };

		// World units across one grid square, and the quantum snapping rounds
		// to. Both uses are the same number on purpose: a grid you can see has
		// to be the grid you snap to.
		float base{ 32.0f };
	};

	Grid grid;
};
