%% ============================
%% レイヤー図（全体構成 + 依存関係）
%% ============================
flowchart TB

    %% アプリケーション層
    subgraph L1["アプリケーション層 (app/)"]
        main["main.cpp\n(エントリーポイント)"]
        loop["GameLoop\n(game_loop.cpp)"]
        updater["GameUpdater\n(game_updater.cpp)"]
        renderer["GameRenderer\n(game_renderer.cpp)"]
        inputH["InputHandler\n(input_handler.cpp)"]
        tutorial["TutorialManager\n(tutorial_manager.cpp)"]
    end

    %% ゲームロジック層
    subgraph L2["ゲームロジック層 (game/)"]
        gs["GameState"]
        stage["StageManager"]
        platform["PlatformSystem"]
        replay["ReplayManager"]
        save["SaveManager"]
        gravity["GravitySystem"]
        swsys["SwitchSystem"]
        cannon["CannonSystem"]
        editor["StageEditor"]
        jsonLoader["JsonStageLoader"]
    end

    %% グラフィックス層
    subgraph L3["グラフィックス層 (gfx/)"]
        oglR["OpenGLRenderer"]
        uiR["UIRenderer"]
        gsUiR["GameStateUIRenderer"]
        cam["CameraSystem"]
        tex["TextureManager"]
        bgR["BackgroundRenderer"]
        r3d["Renderer3D"]
        font["BitmapFont"]
        minimap["MinimapRenderer"]
    end

    %% 物理層
    subgraph L4["物理層 (physics/)"]
        phys["PhysicsSystem"]
    end

    %% 入出力層
    subgraph L5["入出力層 (io/)"]
        inSys["InputSystem"]
        audio["AudioManager"]
    end

    %% コア層
    subgraph L6["コア層 (core/)"]
        consts["Constants"]
        utils["Utils"]
        err["ErrorHandler"]
    end

    %% 外部ライブラリ
    subgraph Ext["外部ライブラリ"]
        GLFW[["GLFW\n(ウィンドウ・入力)"]]
        OpenGL[["OpenGL\n(グラフィックス)"]]
        SDL[["SDL2_mixer\n(オーディオ)"]]
        GLM[["GLM\n(数学ライブラリ)"]]
        JSON[["nlohmann/json\n(JSON処理)"]]
    end

    %% ----------------------------
    %% レイヤ間の依存関係
    %% （上位 → 下位 にのみ依存）
    %% ----------------------------
    main --> loop
    loop --> updater
    loop --> renderer
    loop --> inputH

    %% app → game / gfx / io / core
    L1 --> L2
    L1 --> L3
    L1 --> L5
    L1 --> L6

    %% game → physics / io / core
    L2 --> L4
    L2 --> L5
    L2 --> L6

    %% gfx → core
    L3 --> L6

    %% physics → core
    L4 --> L6

    %% io → core
    L5 --> L6

    %% ----------------------------
    %% 外部ライブラリへの依存
    %% ----------------------------

    %% GLFW: ウィンドウ・入力
    main --> GLFW
    inSys --> GLFW
    inputH --> GLFW

    %% OpenGL: グラフィックス
    oglR --> OpenGL
    uiR --> OpenGL
    gsUiR --> OpenGL
    bgR --> OpenGL
    r3d --> OpenGL
    renderer --> OpenGL

    %% SDL2_mixer: オーディオ
    audio --> SDL

    %% GLM: 数学
    cam --> GLM
    phys --> GLM
    gravity --> GLM
    updater --> GLM
    renderer --> GLM
    stage --> GLM

    %% nlohmann/json: JSON処理
    save --> JSON
    replay --> JSON
    jsonLoader --> JSON
