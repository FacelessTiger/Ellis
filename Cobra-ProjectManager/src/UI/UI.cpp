#include "UI.h"

namespace Cobra::UI {

	void ShiftCursorX(float distance)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + distance);
	}

	void ShiftCursorY(float distance)
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + distance);
	}

	void ShiftCursor(float x, float y)
	{
		const ImVec2 cursor = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(cursor.x + x, cursor.y + y));
	}

	ImRect GetItemRect()
	{
		return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	ImRect RectExpanded(const ImRect& rect, float x, float y)
	{
		ImRect result = rect;
		result.Min.x -= x;
		result.Min.y -= y;
		result.Max.x += x;
		result.Max.y += y;
		return result;
	}

	ImRect RectOffset(const ImRect& rect, float x, float y)
	{
		ImRect result = rect;
		result.Min.x += x;
		result.Min.y += y;
		result.Max.x += x;
		result.Max.y += y;
		return result;
	}

	ImRect RectOffset(const ImRect& rect, ImVec2 xy)
	{
		return RectOffset(rect, xy.x, xy.y);
	}

	bool BeginMenubar(const ImRect& barRectangle)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;
		/*if (!(window->Flags & ImGuiWindowFlags_MenuBar))
			return false;*/

		IM_ASSERT(!window->DC.MenuBarAppending);
		ImGui::BeginGroup(); // Backup position on layer 0 // FIXME: Misleading to use a group for that backup/restore
		ImGui::PushID("##menubar");

		const ImVec2 padding = window->WindowPadding;

		// We don't clip with current window clipping rectangle as it is already set to the area below. However we clip with window full rect.
		// We remove 1 worth of rounding to Max.x to that text in long menus and small windows don't tend to display over the lower-right rounded area, which looks particularly glitchy.
		ImRect bar_rect = UI::RectOffset(barRectangle, 0.0f, padding.y);// window->MenuBarRect();
		ImRect clip_rect(IM_ROUND(ImMax(window->Pos.x, bar_rect.Min.x + window->WindowBorderSize + window->Pos.x - 10.0f)), IM_ROUND(bar_rect.Min.y + window->WindowBorderSize + window->Pos.y),
			IM_ROUND(ImMax(bar_rect.Min.x + window->Pos.x, bar_rect.Max.x - ImMax(window->WindowRounding, window->WindowBorderSize))), IM_ROUND(bar_rect.Max.y + window->Pos.y));

		clip_rect.ClipWith(window->OuterRectClipped);
		ImGui::PushClipRect(clip_rect.Min, clip_rect.Max, false);

		// We overwrite CursorMaxPos because BeginGroup sets it to CursorPos (essentially the .EmitItem hack in EndMenuBar() would need something analogous here, maybe a BeginGroupEx() with flags).
		window->DC.CursorPos = window->DC.CursorMaxPos = ImVec2(bar_rect.Min.x + window->Pos.x, bar_rect.Min.y + window->Pos.y);
		window->DC.LayoutType = ImGuiLayoutType_Horizontal;
		window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
		window->DC.MenuBarAppending = true;
		ImGui::AlignTextToFramePadding();
		return true;
	}

	void EndMenubar()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;
		ImGuiContext& g = *GImGui;

		// Nav: When a move request within one of our child menu failed, capture the request to navigate among our siblings.
		if (ImGui::NavMoveRequestButNoResultYet() && (g.NavMoveDir == ImGuiDir_Left || g.NavMoveDir == ImGuiDir_Right) && (g.NavWindow->Flags & ImGuiWindowFlags_ChildMenu))
		{
			// Try to find out if the request is for one of our child menu
			ImGuiWindow* nav_earliest_child = g.NavWindow;
			while (nav_earliest_child->ParentWindow && (nav_earliest_child->ParentWindow->Flags & ImGuiWindowFlags_ChildMenu))
				nav_earliest_child = nav_earliest_child->ParentWindow;
			if (nav_earliest_child->ParentWindow == window && nav_earliest_child->DC.ParentLayoutType == ImGuiLayoutType_Horizontal && (g.NavMoveFlags & ImGuiNavMoveFlags_Forwarded) == 0)
			{
				// To do so we claim focus back, restore NavId and then process the movement request for yet another frame.
				// This involve a one-frame delay which isn't very problematic in this situation. We could remove it by scoring in advance for multiple window (probably not worth bothering)
				const ImGuiNavLayer layer = ImGuiNavLayer_Menu;
				IM_ASSERT(window->DC.NavLayersActiveMaskNext & (1 << layer)); // Sanity check
				ImGui::FocusWindow(window);
				ImGui::SetNavID(window->NavLastIds[layer], layer, 0, window->NavRectRel[layer]);
				g.NavDisableHighlight = true; // Hide highlight for the current frame so we don't see the intermediary selection.
				g.NavDisableMouseHover = g.NavMousePosDirty = true;
				ImGui::NavMoveRequestForward(g.NavMoveDir, g.NavMoveClipDir, g.NavMoveFlags, g.NavMoveScrollFlags); // Repeat
			}
		}

		IM_MSVC_WARNING_SUPPRESS(6011); // Static Analysis false positive "warning C6011: Dereferencing NULL pointer 'window'"
		// IM_ASSERT(window->Flags & ImGuiWindowFlags_MenuBar); // NOTE(Yan): Needs to be commented out because Jay
		IM_ASSERT(window->DC.MenuBarAppending);
		ImGui::PopClipRect();
		ImGui::PopID();
		window->DC.MenuBarOffset.x = window->DC.CursorPos.x - window->Pos.x; // Save horizontal position so next append can reuse it. This is kinda equivalent to a per-layer CursorPos.
		g.GroupStack.back().EmitItem = false;
		ImGui::EndGroup(); // Restore position on layer 0
		window->DC.LayoutType = ImGuiLayoutType_Vertical;
		window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
		window->DC.MenuBarAppending = false;
	}

	void DrawButtonImage(const  std::shared_ptr<Image>& imageNormal, const  std::shared_ptr<Image>& imageHovered, const  std::shared_ptr<Image>& imagePressed, ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed, ImVec2 rectMin, ImVec2 rectMax)
	{
		auto* drawList = ImGui::GetForegroundDrawList();
		if (ImGui::IsItemActive())
			drawList->AddImage((ImTextureID)(uint64_t)imagePressed->GetRendererID(), rectMin, rectMax, ImVec2(0, 1), ImVec2(1, 0), tintPressed);
		else if (ImGui::IsItemHovered())
			drawList->AddImage((ImTextureID)(uint64_t)imagePressed->GetRendererID(), rectMin, rectMax, ImVec2(0, 1), ImVec2(1, 0), tintHovered);
		else
			drawList->AddImage((ImTextureID)(uint64_t)imagePressed->GetRendererID(), rectMin, rectMax, ImVec2(0, 1), ImVec2(1, 0), tintNormal);
	}

	void DrawButtonImage(const  std::shared_ptr<Image>& imageNormal, const  std::shared_ptr<Image>& imageHovered, const  std::shared_ptr<Image>& imagePressed, ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed, ImRect rectangle)
	{
		DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
	}

	void DrawButtonImage(const  std::shared_ptr<Image>& image, ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed, ImVec2 rectMin, ImVec2 rectMax)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectMin, rectMax);
	}

	void DrawButtonImage(const  std::shared_ptr<Image>& image, ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed, ImRect rectangle)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
	}

	void DrawButtonImage(const  std::shared_ptr<Image>& imageNormal, const  std::shared_ptr<Image>& imageHovered, const  std::shared_ptr<Image>& imagePressed, ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed)
	{
		DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	void DrawButtonImage(const  std::shared_ptr<Image>& image, ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	void RenderWindowOuterBorders(ImGuiWindow* window)
	{
		struct ImGuiResizeBorderDef
		{
			ImVec2 InnerDir;
			ImVec2 SegmentN1, SegmentN2;
			float  OuterAngle;
		};

		static const ImGuiResizeBorderDef resize_border_def[4] =
		{
			{ ImVec2(+1, 0), ImVec2(0, 1), ImVec2(0, 0), IM_PI * 1.00f }, // Left
			{ ImVec2(-1, 0), ImVec2(1, 0), ImVec2(1, 1), IM_PI * 0.00f }, // Right
			{ ImVec2(0, +1), ImVec2(0, 0), ImVec2(1, 0), IM_PI * 1.50f }, // Up
			{ ImVec2(0, -1), ImVec2(1, 1), ImVec2(0, 1), IM_PI * 0.50f }  // Down
		};

		auto GetResizeBorderRect = [](ImGuiWindow* window, int border_n, float perp_padding, float thickness)
			{
				ImRect rect = window->Rect();
				if (thickness == 0.0f)
				{
					rect.Max.x -= 1;
					rect.Max.y -= 1;
				}
				if (border_n == ImGuiDir_Left) { return ImRect(rect.Min.x - thickness, rect.Min.y + perp_padding, rect.Min.x + thickness, rect.Max.y - perp_padding); }
				if (border_n == ImGuiDir_Right) { return ImRect(rect.Max.x - thickness, rect.Min.y + perp_padding, rect.Max.x + thickness, rect.Max.y - perp_padding); }
				if (border_n == ImGuiDir_Up) { return ImRect(rect.Min.x + perp_padding, rect.Min.y - thickness, rect.Max.x - perp_padding, rect.Min.y + thickness); }
				if (border_n == ImGuiDir_Down) { return ImRect(rect.Min.x + perp_padding, rect.Max.y - thickness, rect.Max.x - perp_padding, rect.Max.y + thickness); }
				IM_ASSERT(0);
				return ImRect();
			};


		ImGuiContext& g = *GImGui;
		float rounding = window->WindowRounding;
		float border_size = 1.0f; // window->WindowBorderSize;
		if (border_size > 0.0f && !(window->Flags & ImGuiWindowFlags_NoBackground))
			window->DrawList->AddRect(window->Pos, { window->Pos.x + window->Size.x,  window->Pos.y + window->Size.y }, ImGui::GetColorU32(ImGuiCol_Border), rounding, 0, border_size);

		int border_held = window->ResizeBorderHeld;
		if (border_held != -1)
		{
			const ImGuiResizeBorderDef& def = resize_border_def[border_held];
			ImRect border_r = GetResizeBorderRect(window, border_held, rounding, 0.0f);
			ImVec2 p1 = ImLerp(border_r.Min, border_r.Max, def.SegmentN1);
			const float offsetX = def.InnerDir.x * rounding;
			const float offsetY = def.InnerDir.y * rounding;
			p1.x += 0.5f + offsetX;
			p1.y += 0.5f + offsetY;

			ImVec2 p2 = ImLerp(border_r.Min, border_r.Max, def.SegmentN2);
			p2.x += 0.5f + offsetX;
			p2.y += 0.5f + offsetY;

			window->DrawList->PathArcTo(p1, rounding, def.OuterAngle - IM_PI * 0.25f, def.OuterAngle);
			window->DrawList->PathArcTo(p2, rounding, def.OuterAngle, def.OuterAngle + IM_PI * 0.25f);
			window->DrawList->PathStroke(ImGui::GetColorU32(ImGuiCol_SeparatorActive), 0, ImMax(2.0f, border_size)); // Thicker than usual
		}
		if (g.Style.FrameBorderSize > 0 && !(window->Flags & ImGuiWindowFlags_NoTitleBar) && !window->DockIsActive)
		{
			float y = window->Pos.y + window->TitleBarHeight() - 1;
			window->DrawList->AddLine(ImVec2(window->Pos.x + border_size, y), ImVec2(window->Pos.x + window->Size.x - border_size, y), ImGui::GetColorU32(ImGuiCol_Border), g.Style.FrameBorderSize);
		}
	}
}