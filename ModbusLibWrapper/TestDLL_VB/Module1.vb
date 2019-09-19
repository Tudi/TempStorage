Module Module1

    <System.Runtime.InteropServices.DllImport("ModBusAPI.dll", SetLastError:=False)>
    Public Function GetRegisterValue(ByVal Register As Integer) As Integer
    End Function

    Sub Main()
        Dim value As Integer = GetRegisterValue(0)
        Console.WriteLine("Register value : {0}", value)
    End Sub

End Module
