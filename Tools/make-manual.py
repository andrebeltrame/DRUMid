#!/usr/bin/env python3
"""Builds the illustrated DRUMid manual as a PDF.

    python3 tools/make-manual.py [pt|en] [output.pdf]

The screenshots come from the offscreen preview renderer, so the manual is
generated from the real interface rather than from a mockup that drifts out of
date. Regenerate the shots first when the UI changes:

    cmake --build build --target DrumidPreview && ./build/.../DrumidPreview docs

Callout coordinates below are in the editor's own logical points, taken from the
layout in PluginEditor::resized and StepGrid. The shots are rendered at 2x, so
everything is doubled on the way into the image.
"""

import os
import sys

from PIL import Image, ImageDraw, ImageFont
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.units import mm
from reportlab.lib.enums import TA_LEFT
from reportlab.pdfgen import canvas as pdfcanvas
from reportlab.platypus import Paragraph

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DOCS = os.path.join(ROOT, "docs")
BUILD = os.path.join(ROOT, "build", "manual")

SHOT = os.path.join(DOCS, "drumid-organic-house.png")
SCALE = 2                      # the preview renders at 2x
EDITOR_W, EDITOR_H = 1020, 552

INK = (0.10, 0.11, 0.14)
DIM = (0.42, 0.45, 0.52)
ACCENT = (0.85, 0.48, 0.06)
RULE = (0.85, 0.87, 0.90)
DARK = (0.078, 0.086, 0.102)

FONT_B = "/System/Library/Fonts/Supplemental/Arial Bold.ttf"
FONT_R = "/System/Library/Fonts/Supplemental/Arial.ttf"


# ---------------------------------------------------------------- annotation

def annotate(crop_box, markers, out_name, marker_r=None):
    """Crop the screenshot and stamp numbered markers on it.

    crop_box and marker positions are in editor points; both are scaled here so
    the callout list in the PDF and the layout in the source stay in the same
    units.
    """
    img = Image.open(SHOT).convert("RGBA")

    box = tuple(int(v * SCALE) for v in crop_box)
    crop = img.crop(box)

    # Every crop is printed at the same width on the page, so the marker radius
    # is derived from the crop width. A fixed pixel radius would come out tiny
    # on a full-width strip and enormous on a close-up.
    crop_w_pts = crop_box[2] - crop_box[0]
    r = (marker_r if marker_r is not None else crop_w_pts / 64.0) * SCALE

    layer = Image.new("RGBA", crop.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    font = ImageFont.truetype(FONT_B, max(8, int(r * 1.15)))

    for n, (x, y) in enumerate(markers, start=1):
        cx = x * SCALE - box[0]
        cy = y * SCALE - box[1]

        # A dark halo first, so a marker sitting on an amber block still reads.
        halo = r + max(2.0, r * 0.16)
        d.ellipse([cx - halo, cy - halo, cx + halo, cy + halo], fill=(10, 12, 16, 215))
        d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(255, 177, 61, 255))

        label = str(n)
        tb = d.textbbox((0, 0), label, font=font)
        d.text((cx - (tb[2] - tb[0]) / 2 - tb[0], cy - (tb[3] - tb[1]) / 2 - tb[1]),
               label, font=font, fill=(20, 22, 26, 255))

    out = Image.alpha_composite(crop, layer).convert("RGB")
    os.makedirs(BUILD, exist_ok=True)
    path = os.path.join(BUILD, out_name)
    out.save(path)
    return path


# ---------------------------------------------------------------- pdf helpers

class Manual:
    def __init__(self, path, strings):
        self.c = pdfcanvas.Canvas(path, pagesize=A4)
        self.W, self.H = A4
        self.margin = 18 * mm
        self.y = self.H - self.margin
        self.s = strings
        self.page = 0

    # -- text styles
    def _style(self, size, leading, colour, bold=False, space=0):
        return ParagraphStyle(
            "s", fontName="Helvetica-Bold" if bold else "Helvetica",
            fontSize=size, leading=leading, textColor=colour,
            alignment=TA_LEFT, spaceAfter=space)

    def _draw_para(self, text, style, width=None, indent=0):
        width = width or (self.W - 2 * self.margin - indent)
        p = Paragraph(text, style)
        w, h = p.wrap(width, self.H)
        if self.y - h < self.margin + 14 * mm:
            self.new_page()
            w, h = p.wrap(width, self.H)
        p.drawOn(self.c, self.margin + indent, self.y - h)
        self.y -= h
        return h

    # -- page furniture
    def new_page(self):
        if self.page > 0:
            self.footer()
            self.c.showPage()
        self.page += 1
        self.y = self.H - self.margin

    def footer(self):
        self.c.setFont("Helvetica", 7.5)
        self.c.setFillColorRGB(*DIM)
        self.c.drawString(self.margin, self.margin - 6 * mm, self.s["footer"])
        self.c.drawRightString(self.W - self.margin, self.margin - 6 * mm, str(self.page))

    def cover(self):
        self.new_page()
        c = self.c
        c.setFillColorRGB(*DARK)
        c.rect(0, 0, self.W, self.H, fill=1, stroke=0)

        c.setFillColorRGB(1, 0.694, 0.239)
        c.setFont("Helvetica-Bold", 54)
        c.drawString(self.margin, self.H - 62 * mm, "DRUMid")

        c.setFillColorRGB(0.85, 0.87, 0.91)
        c.setFont("Helvetica-Bold", 13)
        c.drawString(self.margin, self.H - 72 * mm, self.s["subtitle"])

        c.setFillColorRGB(0.48, 0.52, 0.59)
        c.setFont("Helvetica", 10)
        c.drawString(self.margin, self.H - 79 * mm, self.s["cover_line"])

        img = Image.open(SHOT)
        iw = self.W - 2 * self.margin
        ih = iw * img.height / img.width
        c.drawImage(SHOT, self.margin, self.H - 92 * mm - ih, iw, ih,
                    preserveAspectRatio=True, mask=None)

        c.setFillColorRGB(0.48, 0.52, 0.59)
        c.setFont("Helvetica", 9)
        c.drawString(self.margin, self.margin, "Nowhr Dynamics")

    def h1(self, text):
        self.y -= 4 * mm
        self._draw_para(text, self._style(19, 23, ACCENT, bold=True, space=2))
        self.y -= 2 * mm
        self.c.setStrokeColorRGB(*RULE)
        self.c.setLineWidth(0.7)
        self.c.line(self.margin, self.y, self.W - self.margin, self.y)
        self.y -= 5 * mm

    def h2(self, text):
        self.y -= 4 * mm
        self._draw_para(text, self._style(12, 15, INK, bold=True))
        self.y -= 2 * mm

    def body(self, text):
        self._draw_para(text, self._style(9.6, 14, INK))
        self.y -= 3 * mm

    def note(self, text):
        """A short aside, set apart by an accent rule down its left edge."""
        style = self._style(9, 13, DIM)
        p = Paragraph(text, style)
        w, h = p.wrap(self.W - 2 * self.margin - 6 * mm, self.H)
        if self.y - h < self.margin + 14 * mm:
            self.new_page()
            w, h = p.wrap(self.W - 2 * self.margin - 6 * mm, self.H)
        self.c.setStrokeColorRGB(*ACCENT)
        self.c.setLineWidth(2)
        self.c.line(self.margin, self.y - h, self.margin, self.y)
        p.drawOn(self.c, self.margin + 6 * mm, self.y - h)
        self.y -= h + 4 * mm

    def image(self, path, caption=None):
        img = Image.open(path)
        iw = self.W - 2 * self.margin
        ih = iw * img.height / img.width
        if self.y - ih < self.margin + 16 * mm:
            self.new_page()
        self.c.drawImage(path, self.margin, self.y - ih, iw, ih,
                         preserveAspectRatio=True, mask=None)
        self.y -= ih + 3 * mm
        if caption:
            self._draw_para(caption, self._style(8, 11, DIM))
            self.y -= 3 * mm

    def callouts(self, items):
        """Numbered entries matching the markers stamped on the image."""
        for n, (name, text) in enumerate(items, start=1):
            if self.y < self.margin + 24 * mm:
                self.new_page()

            top = self.y
            r = 3.1 * mm
            cx = self.margin + r
            cy = top - r - 0.6 * mm

            self.c.setFillColorRGB(*ACCENT)
            self.c.circle(cx, cy, r, fill=1, stroke=0)
            self.c.setFillColorRGB(1, 1, 1)
            self.c.setFont("Helvetica-Bold", 7.5)
            self.c.drawCentredString(cx, cy - 2.6, str(n))

            self._draw_para(f"<b>{name}</b>  {text}",
                            self._style(9.4, 13.2, INK), indent=10 * mm)
            self.y -= 2.4 * mm

    def table(self, rows, col=52 * mm):
        for left, right in rows:
            if self.y < self.margin + 20 * mm:
                self.new_page()
            top = self.y
            hl = self._draw_para(left, self._style(9.4, 13, INK, bold=True),
                                 width=col - 4 * mm)
            self.y = top
            p = Paragraph(right, self._style(9.4, 13, DIM))
            w, h = p.wrap(self.W - 2 * self.margin - col, self.H)
            p.drawOn(self.c, self.margin + col, self.y - h)
            self.y -= max(hl, h) + 2.2 * mm

    def save(self):
        self.footer()
        self.c.save()


# ---------------------------------------------------------------- content

# Editor points, straight out of PluginEditor::resized and StepGrid.
CROP_TOP     = (0, 0, 1020, 50)
CROP_CONTROL = (0, 44, 1020, 124)
CROP_LANE    = (0, 110, 300, 245)
CROP_GRID    = (250, 122, 1020, 282)

MARK_TOP     = [(60, 24), (237, 24), (360, 24), (478, 24), (965, 24)]
MARK_CONTROL = [(77, 69), (77, 104), (150, 60), (583, 83), (946, 83)]
MARK_LANE    = [(29, 120), (87, 120), (147, 120), (188, 120), (223, 120), (245, 120)]

PT = {
    "file": "DRUMid-1.0.0-Manual.pdf",
    "subtitle": "Gerador de padrões de bateria em MIDI",
    "cover_line": "Manual ilustrado — versão 1.0.0",
    "footer": "DRUMid 1.0.0 — Nowhr Dynamics",

    "flow_h": "Em quatro passos",
    "flow": [
        ("1. Escolha o gênero",
         "Organic house, afro house, indie dance, melodic house, melodic techno ou techno. "
         "A lista está ordenada do mais solto ao mais duro, então percorrê-la já é um movimento musical."),
        ("2. Ligue os instrumentos que quiser",
         "Clique no nome de uma lane para desligá-la. Perc 2 vem desligada; é uma voz extra opcional."),
        ("3. GENERATE",
         "Sorteia os padrões de todas as lanes ligadas e destravadas. Gostou de uma? Trave com L e "
         "aperte de novo — só o resto muda."),
        ("4. Arraste o MIDI",
         "Arraste DRAG KIT MIDI para o Ableton, ou o ícone de uma lane para levar só aquele instrumento."),
    ],
    "flow_note": "O DRUMid não produz som nenhum. Ele escreve os padrões e entrega o MIDI — "
                 "quem toca são os seus racks e samplers.",

    "top_h": "A barra superior",
    "top": [
        ("Nome e versão",
         "Clique para abrir a janela About, onde ficam a versão e o número de padrões do banco."),
        ("Gênero",
         "Trocar o gênero regenera tudo e ajusta o swing para o feel natural daquele estilo — "
         "techno em 50% (reto), organic e afro house em 56%."),
        ("Compassos",
         "1, 2 ou 4. Em 2 ou mais compassos o segundo não é cópia do primeiro: os fantasmas se deslocam "
         "e a última frase ganha uma virada."),
        ("Mapa de notas",
         "Em que nota cada instrumento cai. Veja a seção sobre o Ableton — é a decisão que define seu fluxo."),
        ("Seed",
         "O número que gerou este kit. O mesmo seed sempre reconstrói o mesmo resultado, e é isso que "
         "faz travar e re-sortear funcionar."),
    ],

    "ctrl_h": "A faixa de controles",
    "ctrl": [
        ("GENERATE",
         "Sorteia os padrões usando as configurações atuais."),
        ("SURPRISE",
         "Sorteia também as configurações — gênero, energia, complexidade, feel e a dinâmica de cada "
         "elemento. Lanes travadas e a quantidade de compassos sobrevivem aos dois botões."),
        ("Os cinco knobs",
         "Descritos na tabela abaixo."),
        ("Fills",
         "Liga a virada no fim da frase: rufo de tom entrando no downbeat, hats adensando nos últimos "
         "16 avos, e às vezes o último kick sai."),
        ("DRAG KIT MIDI",
         "Arraste para o Ableton. O que sai depende do mapa de notas — um clip só, ou um track por "
         "instrumento."),
    ],
    "knobs_h": "Os cinco knobs",
    "knobs": [
        ("ENERGY",
         "O quão cheio é o kit. É um orçamento <b>compartilhado entre as lanes</b>, não um ajuste por lane — "
         "senão cada instrumento decide sozinho o quanto tocar e o resultado vira lama. Kick e clap são o "
         "esqueleto e nunca são cortados; quem cede é decidido por gênero: em afro house a conga ganha e o "
         "hi-hat recua, em techno é o contrário."),
        ("COMPLEX",
         "Síncope, notas fantasma e ratchets (rufos de 32 avos dentro de um step)."),
        ("SWING",
         "50% é reto. Cada gênero começa no seu feel natural; a partir daí é seu. O swing vira microtiming "
         "de verdade e vai gravado no MIDI exportado — não é uma aproximação quantizada."),
        ("TIMING",
         "Humanização do tempo. O kick recebe só um quarto disso: arrastar o kick soa quebrado, não humano."),
        ("DYNAMICS",
         "A quantidade global de variação de velocity. O DYN de cada lane multiplica este valor."),
    ],

    "lane_h": "O cabeçalho de cada instrumento",
    "lane": [
        ("Ícone",
         "<b>Arraste</b> para exportar só aquele instrumento em MIDI."),
        ("Nome",
         "Clique para ligar ou desligar a lane. Não existe mute nem solo: cada lane toca o seu próprio "
         "instrumento, então ligar e desligar é o único interruptor que faz sentido aqui."),
        ("Nota",
         "Arraste para cima ou para baixo para mudar a nota. Isso troca o mapa para Custom."),
        ("DYN",
         "O quanto a velocity <i>deste</i> elemento passeia, como multiplicador do knob DYNAMICS. "
         "100% é exatamente a quantidade global; 0% congela o elemento; 200% é o dobro. "
         "Os padrões seguem o instrumento — 35% no kick, 140% no shaker e nas congas — porque dar a mesma "
         "quantidade a todos é justamente o que denuncia bateria programada."),
        ("L — travar",
         "O GENERATE não toca nesta lane. Trave o que você gostou e sorteie o resto."),
        ("R — refazer",
         "Refaz só este instrumento, inclusive a velocity."),
    ],

    "grid_h": "A grade",
    "grid_intro": "Cada coluna é um 16 avos. A altura do bloco é a velocity, e o bloco aparece deslocado "
                  "pelo microtiming real — então o swing e a humanização são coisas que você <i>vê</i>, "
                  "não só ouve.",
    "grid_tbl": [
        ("clique / arraste", "pinta os steps, ligando e desligando"),
        ("alt + arraste", "velocity daquele step"),
        ("clique duplo", "alterna o ratchet: 1 → 2 → 3"),
        ("blocos escuros", "notas fantasma — sopradas, e com probabilidade de não soar"),
        ("traços dentro do bloco", "ratchet: o step foi subdividido"),
        ("contorno claro", "aquele step tem probabilidade menor que 100%"),
    ],

    "able_h": "Levando o MIDI para o Ableton",
    "able_a_h": "Um instrumento por canal (o padrão)",
    "able_a": "Todas as lanes ficam em C3, que é a nota raiz do Simpler e do Sampler. Aqui quem separa os "
              "instrumentos é o roteamento, não a nota. Arraste DRAG KIT MIDI para a Session View e o Live "
              "cria <b>um canal por instrumento</b>, já nomeado Kick, Clap, Tom e assim por diante. Para "
              "levar um instrumento só, arraste o ícone da lane.",
    "able_b_h": "Tocando um Drum Rack",
    "able_b": "Troque o mapa para GM / Drum Rack. Aí cada lane ganha a sua própria nota — 36 kick, 39 clap, "
              "42 closed hat — e o drag do kit escreve um track só, que cai como um clip único tocando o "
              "rack inteiro.",
    "able_c_h": "Ouvindo ao vivo, sem exportar",
    "able_c": "O DRUMid é um instrumento que emite MIDI, então ele vai num canal MIDI próprio. No canal do "
              "seu rack: <b>MIDI From</b> → o canal do DRUMid → <b>DRUMid</b>, e <b>Monitor: In</b>.",
    "able_note": "Não há botão de play. O DRUMid segue o transporte do host e nada mais — ele é um "
                 "escritor de MIDI, não um tocador com relógio próprio.",

    "genre_h": "O que cada gênero sabe",
    "genres": [
        ("Organic house",
         "A percussão de mão carrega: tresillo, clave son 3-2, clave rumba 3-2, E(5,16) e E(7,16). "
         "Clap muitas vezes só no 4. Swing mais pesado e variação de velocity bem mais larga — é isso que "
         "faz soar tocado em vez de programado."),
        ("Afro house",
         "A percussão <i>é</i> a faixa: conga rolante, tumbao, cáscara, bongô em E(7,16), um 16 avos "
         "fantasma antes do downbeat, tom tribal respondendo."),
        ("Indie dance",
         "Tocado, não programado: hats em 8 avos com shuffle de verdade, caixas fantasma, tamborim e "
         "cowbell no contratempo, kicks quebrados, viradas de tom."),
        ("Melodic house",
         "Open hat nos contratempos de 8 avos, hats de 16 avos com o acento no contratempo, swing leve, "
         "ghost de kick entrando no compasso."),
        ("Melodic techno",
         "Reto e paciente: hat no contratempo, backbeat de cauda longa, toms graves isolados, ticks "
         "metálicos esparsos em 3-contra-4."),
        ("Techno",
         "Reto, swing zero, closed hat só no contratempo — essa colocação <i>é</i> o gênero. Metal "
         "sincopado entre os kicks, toms rolando em 8 avos, rufos de 32 avos."),
    ],

    "ref_h": "Referência rápida",
    "ref": [
        ("GENERATE", "sorteia os padrões"),
        ("SURPRISE", "sorteia gênero, configurações e a dinâmica de cada elemento, e gera"),
        ("nome da lane", "liga / desliga o instrumento"),
        ("arraste o ícone", "exporta só aquela lane em MIDI"),
        ("arraste a nota", "muda a nota da lane (vira mapa Custom)"),
        ("arraste DYN", "o quanto a velocity deste elemento passeia"),
        ("L", "trava a lane contra o GENERATE"),
        ("R", "refaz só este instrumento"),
        ("clique / arraste na grade", "pinta os steps"),
        ("alt + arraste", "velocity do step"),
        ("clique duplo", "alterna o ratchet"),
        ("DRAG KIT MIDI", "arrasta o kit inteiro para o Ableton"),
    ],
}


def build(strings, out_path):
    top = annotate(CROP_TOP, MARK_TOP, "shot-top.png")
    ctrl = annotate(CROP_CONTROL, MARK_CONTROL, "shot-controls.png")
    lane = annotate(CROP_LANE, MARK_LANE, "shot-lane.png")
    grid = annotate(CROP_GRID, [], "shot-grid.png")

    m = Manual(out_path, strings)
    m.cover()

    m.new_page()
    m.h1(strings["flow_h"])
    for name, text in strings["flow"]:
        m.h2(name)
        m.body(text)
    m.note(strings["flow_note"])

    m.h1(strings["top_h"])
    m.image(top)
    m.callouts(strings["top"])

    m.new_page()
    m.h1(strings["ctrl_h"])
    m.image(ctrl)
    m.callouts(strings["ctrl"])
    m.h2(strings["knobs_h"])
    m.table(strings["knobs"], col=30 * mm)

    m.new_page()
    m.h1(strings["lane_h"])
    m.image(lane)
    m.callouts(strings["lane"])

    m.new_page()
    m.h1(strings["grid_h"])
    m.body(strings["grid_intro"])
    m.image(grid)
    m.table(strings["grid_tbl"], col=44 * mm)

    m.new_page()
    m.h1(strings["able_h"])
    m.h2(strings["able_a_h"])
    m.body(strings["able_a"])
    m.h2(strings["able_b_h"])
    m.body(strings["able_b"])
    m.h2(strings["able_c_h"])
    m.body(strings["able_c"])
    m.note(strings["able_note"])

    m.h1(strings["genre_h"])
    m.table(strings["genres"], col=34 * mm)

    m.new_page()
    m.h1(strings["ref_h"])
    m.table(strings["ref"], col=52 * mm)

    m.save()
    return out_path


if __name__ == "__main__":
    lang = sys.argv[1] if len(sys.argv) > 1 else "pt"
    strings = PT
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(DOCS, strings["file"])
    print(build(strings, out))
