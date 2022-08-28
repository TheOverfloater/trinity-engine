// Tracker scheme resource file
Scheme
{
	// default settings for all panels
	BaseSettings
	{
		"FgColor"			"255 170 0 255"
		"BgColor"			"0 0 0 160"
		"LabelBgColor"		"0 0 0 0"
		"SubPanelBgColor"	"0 0 0 0"

		"FgColorDim"		"178 119 0 255"

		"DisabledFgColor1"		"80 80 80 255" 
		"DisabledFgColor2"		"40 40 40 255"	// set this to the BgColor if you don't want it to draw

		"TitleBarFgColor"			"0 0 0 255"
		"TitleBarDisabledFgColor"	"150 150 150 255"
		"TitleBarBgColor"			"180 120 0 192"
		"TitleBarDisabledBgColor"	"99 99 99 192"

		"TitleBarIcon"				"icon_tracker"
		"TitleBarDisabledIcon"		"icon_tracker_disabled"

		"TitleButtonFgColor"			"97 64 0 255"
		"TitleButtonBgColor"			"197 131 0 160"
		"TitleButtonDisabledFgColor"	"70 70 70 255"
		"TitleButtonDisabledBgColor"	"99 99 99 160"

		"TextCursorColor"			"0 0 0 255"
		"URLTextColor"				"200 200 240 255"

		"BrightControlText"	"255 170 0 255"		// half-life orange

		Menu
		{
			"FgColor"			"150 150 150 255"
			"BgColor"			"0 0 0 192"
			"ArmedFgColor"		"255 183 0 255"
			"ArmedBgColor"		"134 91 19 192"
			"DividerColor"		"56 56 56 255"

			"TextInset"			"6"			//!! not working yet
		}

		 MenuButton	  // the little arrow on the side of boxes that triggers drop down menus
		{
			"ButtonArrowColor"	"220 220 220 255"	// color of arrows
		   	"ButtonBgColor"		"80 80 80 255"	// bg color of button. same as background color of text edit panes 
			
			"ArmedArrowColor"		"255 170 0 255" // color of arrow when mouse is over button
			"ArmedBgColor"		"150 150 150 205"  // bg color of button when mouse is over button

		}

		ScrollBar
		{
			"BgColor"			"170 70 70 0"
			"SliderFgColor"		"0 0 0 255"
			"SliderBgColor"		"40 40 40 255"

			"ButtonFgColor"		"140 140 140 255"
		}

		// text edit windows
//		"WindowFgColor"				"255 170 0 255"		// half-life orange
		"WindowFgColor"				"220 220 220 255"		// off-white
		"WindowBgColor"				"100 100 100 205"
		"WindowDisabledFgColor"		"150 150 150 205"
		"WindowDisabledBgColor"		"70 70 70 160"

		"SelectionBgColor"			"134 91 19 255"
		
		// App-specific stuff

		// status selection
		"StatusSelectFgColor"		"255 255 255 255"
		"StatusSelectFgColor2"		"121 121 121 255"

		// buddy buttons
		BuddyButton
		{
			"FgColor1"		"255 183 0 255"
			"FgColor2"		"178 119 0 255"

			"ArmedFgColor1"	"255 183 0 255"
			"ArmedFgColor2"	"255 183 0 255"
			"ArmedBgColor"	"134 91 19 192"
		}

		Chat
		{
			"TextColor"				"255 170 0 255"
			"SelfTextColor"			"180 120 0 255"
			"SeperatorTextColor"	"121 121 121 255"
		}

		"SectionTextColor"		"121 121 121 255"
		"SectionDividerColor"	"70 70 70 255"
	}

	// describes all the fonts
	Fonts
	{
		"Default"
		{
			"name"		"Tahoma"
			"tall"		"16"
			"weight"	"500"
		}

		"DefaultUnderline"
		{
			"name"		"Tahoma"
			"tall"		"16"
			"weight"	"500"
			"underline" "1"
		}

		"DefaultSmall"
		{
			"name"		"Tahoma"
			"tall"		"13"
			"weight"	"0"
		}

		"DefaultVerySmall"
		{
			"name"		"Tahoma"
			"tall"		"12"
			"weight"	"0"
		}

		// this is the symbol font
		"Marlett"
		{
			"name"		"Marlett"
			"tall"		"14"
			"weight"	"0"
		}
	}

	// describes all the images used
	Images
	{
		"logo_valve"
		{
			"filename"	"resource\\logo_valve"
			"width"		"66"
			"height"	"18"
		}

		"icon_tracker"
		{
			"filename"	"resource\\icon_tracker"
			"width"		"16"
			"height"	"16"
		}
		"icon_tracker_disabled"
		{
			"filename"	"resource\\icon_tracker_disabled"
			"width"		"16"
			"height"	"16"
		}
		"icon_tray"
		{
			"filename"	"resource\\icon_tracker"
			"width"		"16"
			"height"	"16"
		}
		"icon_away"
		{
			"filename"	"resource\\icon_away"
			"width"		"16"
			"height"	"16"
		}
		"icon_busy"
		{
			"filename"	"resource\\icon_busy"
			"width"		"16"
			"height"	"16"
		}
		"icon_blocked"
		{
			"filename"	"resource\\icon_blocked"
			"width"		"16"
			"height"	"16"
		}
		"icon_online"
		{
			"filename"	"resource\\icon_blank"
			"width"		"16"
			"height"	"16"
		}
		"icon_offline"
		{
			"filename"	"resource\\icon_blank"
			"width"		"16"
			"height"	"16"
		}
		"icon_connecting"
		{
			"filename"	"resource\\icon_blank"
			"width"		"16"
			"height"	"16"
		}
		"icon_message"
		{
			"filename"	"resource\\icon_message"
			"width"		"16"
			"height"	"16"
		}
		"icon_blank"
		{
			"filename"	"resource\\icon_blank"
			"width"		"16"
			"height"	"16"
		}
		"icon_snooze"
		{
			"filename"	"resource\\icon_snooze"
			"width"		"16"
			"height"	"16"
		}
		"icon_password"
		{
			"filename"	"resource\\icon_password"
			"width"		"16"
			"height"	"16"
		}
	}

	// describes all the border types
	Borders
	{
		BaseBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "0 0"
				}
			}
		}
		
		TitleButtonBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "255 170 0 255"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "97 64 0 255"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "255 170 0 255"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "97 64 0 255"
					"offset" "0 0"
				}
			}
		}

		TitleButtonDisabledBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "90 90 90 255"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "70 70 70 255"
					"offset" "1 0"
				}
			}
			Top
			{
				"1"
				{
					"color" "90 90 90 255"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "70 70 70 255"
					"offset" "0 0"
				}
			}
		}

		TitleButtonDepressedBorder
		{
			"inset" "1 1 1 1"
			Left
			{
				"1"
				{
					"color" "97 64 0 255"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "255 170 0 255"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "97 64 0 255"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "255 170 0 255"
					"offset" "0 0"
				}
			}
		}

		ScrollBarButtonBorder
		{
			"inset" "2 2 1 1"
			Left
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
			}
		}

		ButtonBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
			}
		}

		TabBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "0 0"
				}
			}
		}

		TabActiveBorder
		{
			"inset" "0 0 1 0"
			Left
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "0 0"
				}
			}

			Right
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "70 70 70 255"
					"offset" "6 0"
				}
			}
		}

		// this is the border used for default buttons (the button that gets pressed when you hit enter)
		ButtonKeyFocusBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
				"2"
				{
					"color" "99 99 99 255"
					"offset" "0 1"
				}
			}
			Top
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
				"2"
				{
					"color" "99 99 99 255"
					"offset" "1 0"
				}
			}
			Right
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
				"2"
				{
					"color" "0 0 0 255"
					"offset" "1 0"
				}
			}
			Bottom
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
				"2"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
			}
		}

		ButtonDepressedBorder
		{
			"inset" "2 1 1 1"
			Left
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "0 0"
				}
			}
		}

		ComboBoxBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "0 0"
				}
			}
		}

		MenuBorder
		{
			"inset" "1 1 1 1"
			Left
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "99 99 99 255"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
			}
		}
	}
}