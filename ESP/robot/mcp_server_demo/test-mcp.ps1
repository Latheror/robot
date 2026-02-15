# MCP Server Test Script
$init = @{
    jsonrpc = "2.0"
    id = 1
    method = "initialize"
    params = @{
        protocolVersion = "2025-01-01"
        capabilities = @{}
        clientInfo = @{
            name = "test-client"
            version = "1.0.0"
        }
    }
} | ConvertTo-Json -Compress

$list = @{
    jsonrpc = "2.0"
    id = 2
    method = "tools/list"
    params = @{}
} | ConvertTo-Json -Compress

$call = @{
    jsonrpc = "2.0"
    id = 3
    method = "tools/call"
    params = @{
        name = "add"
        arguments = @{
            a = 5
            b = 3
        }
    }
} | ConvertTo-Json -Compress

$call_weather = @{
    jsonrpc = "2.0"
    id = 4
    method = "tools/call"
    params = @{
        name = "getweather"
        arguments = @{}
    }
} | ConvertTo-Json -Compress

# Send requests to MCP server
@($init, $list, $call, $call_weather) -join [char]10 | node build/index.js stdio
