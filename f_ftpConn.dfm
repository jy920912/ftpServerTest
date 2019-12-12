object FtpConn: TFtpConn
  Left = 0
  Top = 0
  AutoSize = True
  BorderIcons = []
  Caption = #50629#47196#46300' '#51473'...'
  ClientHeight = 180
  ClientWidth = 400
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 0
    Top = 40
    Width = 400
    Height = 100
    Margins.Left = 0
    Margins.Top = 0
    Margins.Right = 0
    Margins.Bottom = 0
    BevelOuter = bvNone
    Color = clBtnHighlight
    ParentBackground = False
    TabOrder = 0
    object Label2: TLabel
      Left = 17
      Top = 3
      Width = 370
      Height = 15
      AutoSize = False
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = #46027#50880
      Font.Style = []
      ParentFont = False
    end
    object Label3: TLabel
      Left = 17
      Top = 48
      Width = 64
      Height = 15
      AutoSize = False
      Caption = #51204#52404' '#54028#51068
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = #46027#50880
      Font.Style = []
      ParentFont = False
    end
    object Label4: TLabel
      Left = 73
      Top = 48
      Width = 28
      Height = 13
      Caption = '(0/0)'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = #46027#50880
      Font.Style = []
      ParentFont = False
    end
    object ProgressBar1: TProgressBar
      Left = 17
      Top = 23
      Width = 370
      Height = 20
      Max = 10000
      TabOrder = 0
    end
    object ProgressBar2: TProgressBar
      Left = 17
      Top = 68
      Width = 370
      Height = 20
      Max = 10000
      TabOrder = 1
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 0
    Width = 400
    Height = 40
    Margins.Left = 0
    Margins.Top = 0
    Margins.Right = 0
    Margins.Bottom = 0
    BevelOuter = bvNone
    Color = clGradientInactiveCaption
    ParentBackground = False
    TabOrder = 1
    object Label1: TLabel
      Left = 17
      Top = 13
      Width = 232
      Height = 13
      Caption = #50629#47196#46300#51473#51077#45768#45796'. '#51104#49884#47564' '#44592#45796#47140#51452#49464#50836'...'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = #46027#50880
      Font.Style = []
      ParentFont = False
    end
  end
  object Panel3: TPanel
    Left = 0
    Top = 140
    Width = 400
    Height = 40
    Margins.Left = 0
    Margins.Top = 0
    Margins.Right = 0
    Margins.Bottom = 0
    BevelOuter = bvNone
    ParentBackground = False
    TabOrder = 2
    object Button1: TButton
      Left = 285
      Top = 6
      Width = 91
      Height = 30
      Caption = #52712'    '#49548
      TabOrder = 0
      OnClick = Button1Click
    end
  end
end
