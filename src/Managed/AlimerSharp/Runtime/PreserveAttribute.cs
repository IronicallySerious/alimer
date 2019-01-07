// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

[System.AttributeUsage(System.AttributeTargets.All, AllowMultiple = false)]
public sealed class PreserveAttribute : System.Attribute
{
    public bool AllMembers;
    public bool Conditional;
}
