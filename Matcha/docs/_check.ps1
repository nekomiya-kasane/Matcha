# Check exported HTML — decode base64 and look for srcdoc
$html = [System.IO.File]::ReadAllText("$PSScriptRoot\Matcha_Design_System_Specification.html", [System.Text.Encoding]::UTF8)
$b64Start = $html.IndexOf('type="text/plain">')
if ($b64Start -ge 0) {
    $b64Start += 'type="text/plain">'.Length
    $b64End = $html.IndexOf("</script>", $b64Start)
    $b64 = $html.Substring($b64Start, $b64End - $b64Start).Trim()
    Write-Host "Base64 length: $($b64.Length)"
    $decoded = [System.Text.Encoding]::UTF8.GetString([Convert]::FromBase64String($b64))
    Write-Host "Decoded length: $($decoded.Length)"

    $srcIdx = $decoded.IndexOf('src="playground/components/')
    $srcdocIdx = $decoded.IndexOf('srcdoc=')
    Write-Host "src=playground in decoded: $srcIdx"
    Write-Host "srcdoc= in decoded: $srcdocIdx"

    if ($srcdocIdx -ge 0) {
        Write-Host "SUCCESS: srcdoc found in decoded base64"
        Write-Host $decoded.Substring($srcdocIdx, [Math]::Min(200, $decoded.Length - $srcdocIdx))
    }
}
