using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Automation;

namespace CradleWindowsAgent
{
    class MonitorOpenPanel
    {
        public void StartTracking()
        {
            //tRACK IF FILE UPLOAD WINDOW IS OPENED (FIREFOX TESTED)
            try
            {
                AutomationEventHandler windowClosedHandler = null;
                string ElementText = "File Upload"; //Firefox, in Chrome it is Open
                Automation.AddAutomationEventHandler(WindowPattern.WindowOpenedEvent,
                AutomationElement.RootElement, TreeScope.Subtree, (UIElm, evt) =>
                {
                    AutomationElement element = UIElm as AutomationElement;
                    try
                    {
                        if (element is null) return;

                        // Check if the window title is "File Upload"
                        if (element.Current.Name != "File Upload")
                        {
                            return;
                        }

                        // Specify the class name of the EDIT element
                        string editClassName = "Edit";

                        // Find the EDIT element by class name
                        PropertyCondition editCondition = new PropertyCondition(AutomationElement.ClassNameProperty, editClassName);
                        AutomationElement editElement = element.FindFirst(TreeScope.Descendants, editCondition);

                        if (editElement != null)
                        {
                            // Subscribe to the ValuePattern's property changed event
                            Automation.AddAutomationPropertyChangedEventHandler(editElement, TreeScope.Element, (sender, e) =>
                            {
                                if (e.Property == ValuePattern.ValueProperty)
                                {
                                    // Value has changed, retrieve the new value
                                    object valuePattern;
                                    if (editElement.TryGetCurrentPattern(ValuePattern.Pattern, out valuePattern))
                                    {
                                        string newValue = (valuePattern as ValuePattern).Current.Value;
                                        Console.WriteLine("New value of EDIT element: " + newValue);
                                        ((App)(Application.Current)).wndMain.AddToList("Value of Selected Files element: " + newValue);
                                    }
                                }
                            }, ValuePattern.ValueProperty);
                        }
                        else
                        {
                            // Handle case where EDIT element is not found
                            Console.WriteLine("EDIT element not found.");
                        }

                        // Specify the class name of the ToolbarWindow32 element
                        string toolbarClassName = "ToolbarWindow32";

                        // Find the ToolbarWindow32 element by class name
                        PropertyCondition toolbarCondition = new PropertyCondition(AutomationElement.ClassNameProperty, toolbarClassName);
                        AutomationElement toolbarElement = element.FindFirst(TreeScope.Descendants, toolbarCondition);

                        if (toolbarElement != null)
                        {
                            //Get the initial value:
                            // Try to access NameProperty
                            string name = toolbarElement.Current.Name;
                            if (!string.IsNullOrEmpty(name))
                            {
                                Console.WriteLine("Initial value of ToolbarWindow32 element (NameProperty): " + name);
                            }
                            else
                            {
                                Console.WriteLine("NameProperty is not available for the ToolbarWindow32 element.");
                            }


                            object valuePattern;
                            if (toolbarElement.TryGetCurrentPattern(ValuePattern.Pattern, out valuePattern))
                            {
                                string newValue = (valuePattern as ValuePattern).Current.Value;
                                Console.WriteLine("Initial value of ToolbarWindow32 element: " + newValue);
                            }

                            // Subscribe to the ValuePattern's property changed event
                            Automation.AddAutomationPropertyChangedEventHandler(toolbarElement, TreeScope.Element, (sender, e) =>
                            {
                                if (e.Property == ValuePattern.ValueProperty)
                                {
                                    // Value has changed, retrieve the new value

                                    if (toolbarElement.TryGetCurrentPattern(ValuePattern.Pattern, out valuePattern))
                                    {
                                        string newValue = (valuePattern as ValuePattern).Current.Value;
                                        Console.WriteLine("New value of ToolbarWindow32 element: " + newValue);
                                    }
                                }
                            }, ValuePattern.ValueProperty);

                            // Retrieve the text value using TextPattern
                            TextPattern textPattern = toolbarElement.GetCurrentPattern(TextPattern.Pattern) as TextPattern;
                            if (textPattern != null)
                            {
                                // Get the initial text value
                                string initialValue = textPattern.DocumentRange.GetText(-1);
                                Console.WriteLine("Initial value of ToolbarWindow32 element: " + initialValue);
                            }
                            else
                            {
                                Console.WriteLine("TextPattern is not supported for the ToolbarWindow32 element.");
                            }
                        }
                        else
                        {
                            // Handle case where ToolbarWindow32 element is not found
                            Console.WriteLine("ToolbarWindow32 element not found.");
                        }

                        // Subscribe to the window close event
                        Automation.AddAutomationEventHandler(WindowPattern.WindowClosedEvent,
                            element, TreeScope.Element, (sender, args) =>
                            {
                                // Print the value of the EDIT element before closing the window
                                //We might also need to know if the window was cancelled or it was a real file select event.

                            });

                    }
                    catch (ElementNotAvailableException)
                    {
                        // Ignore: this exception may be raised when a modal dialog owned 
                        // by the current process is shown, blocking the code execution. 
                        // When the dialog is closed, the AutomationElement may no longer be available
                    }
                });

                // Subscribe to the DragEnter event
                //Automation.AddAutomationEventHandler(
                //    AutomationEvent.LookupById(20026),
                //    AutomationElement.FocusedElement,
                //    TreeScope.Subtree,
                //    (o, e) => { Console.WriteLine("Text Changed Event (I want this to fire please)"); });
            }
            catch (Exception e)
            {
                ;
            }
        }

        public void StopTracking()
        {
            // To stop the tracking, remove the event handler
            //Automation.RemoveAutomationEventHandler(WindowPattern.WindowOpenedEvent,
            //    AutomationElement.RootElement, windowOpenedHandler);
        }
    }
}
