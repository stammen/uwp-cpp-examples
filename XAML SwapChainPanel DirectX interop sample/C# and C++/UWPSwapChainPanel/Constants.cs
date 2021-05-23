//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

using System.Collections.Generic;
using System;
using SwapChainPanel;

namespace UWPSwapChainPanel
{
    public partial class MainPage : UWPSwapChainPanel.Common.LayoutAwarePage
    {
        // Change the string below to reflect the name of your sample.
        // This is used on the main page as the title of the sample.
        public const string FEATURE_NAME = "DirectX interop with SwapChainPanel";

        // Change the array below to reflect the name of your scenarios.
        // This will be used to populate the list of scenarios on the main page with
        // which the user will choose the specific scenario that they are interested in.
        // These should be in the form: "Navigating to a web page".
        // The code in MainPage will take care of turning this into: "1) Navigating to a web page"
        List<Scenario> scenarios = new List<Scenario>
        {
            new Scenario() { Title = "SwapChainPanel", ClassType = typeof(Scenario1) },
            new Scenario() { Title = "Independent Input", ClassType = typeof(Scenario2) },
            new Scenario() { Title = "Handling scale changes", ClassType = typeof(Scenario3) },
            new Scenario() { Title = "Highlighting using CompositeMode", ClassType = typeof(Scenario4) },
            new Scenario() { Title = "Accessibility: UI Automation", ClassType = typeof(Scenario5) }
        };
    }

    public class Scenario
    {
        public string Title { get; set; }

        public Type ClassType { get; set; }

        public override string ToString()
        {
            return Title;
        }
    }
}
