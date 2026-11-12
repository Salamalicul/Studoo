#include "AutomationView.h"
#include "HelpTexts.h"

namespace studoo
{
// ============================================================================
//  AutomationEditor
// ============================================================================
AutomationEditor::AutomationEditor (AppContext& c, AutomationView& v) : ctx (c), view (v)
{
    setTooltip (txt ("Clic en un hueco: añadir un punto. Arrastra un punto: moverlo. Clic derecho: borrarlo. "
                     "Arriba = valor máximo, abajo = mínimo."));
    makeNonFocusing (*this);
}

double AutomationEditor::totalSteps()  { return ctx.project.getSongLengthSteps() + 4.0 * stepsPerBar; }
float AutomationEditor::timeToX (double t) { return (float) (t / totalSteps() * getWidth()); }
double AutomationEditor::xToTime (float x) { return juce::jmax (0.0, x / (double) getWidth() * totalSteps()); }
float AutomationEditor::valueToY (float v) const { return ruler + 8.0f + (1.0f - v) * (float) (getHeight() - ruler - 16); }
float AutomationEditor::yToValue (float y) const { return juce::jlimit (0.0f, 1.0f, 1.0f - (y - ruler - 8.0f) / (float) (getHeight() - ruler - 16)); }

juce::ValueTree AutomationEditor::pointAt (juce::Point<float> p)
{
    for (auto pt : view.getLane())
        if (juce::Point<float> (timeToX (pt[ids::time]), valueToY (pt[ids::value])).getDistanceFrom (p) < 8.0f)
            return pt;
    return {};
}

void AutomationEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff22272c));

    const double total = totalSteps();
    const int bars = (int) (total / stepsPerBar);
    g.setFont (juce::FontOptions (11.0f));
    for (int b = 0; b <= bars; ++b)
    {
        const float x = timeToX (b * stepsPerBar);
        g.setColour (Colours::outline);
        g.drawVerticalLine ((int) x, (float) ruler, (float) getHeight());
        if (bars < 60 || b % 2 == 0)
        {
            g.setColour (Colours::textDim);
            g.drawText (juce::String (b + 1), juce::Rectangle<float> (x + 3.0f, 0.0f, 30.0f, (float) ruler), juce::Justification::centredLeft);
        }
    }

    // Zona de la canción y bloques de la playlist como referencia
    for (auto clip : ctx.project.playlist())
    {
        const float x = timeToX (clip[ids::start]);
        const float w = timeToX ((double) clip[ids::start] + (double) clip[ids::length]) - x;
        g.setColour (juce::Colours::white.withAlpha (0.04f));
        g.fillRect (x, (float) ruler, w, (float) getHeight() - ruler);
    }

    auto lane = view.getLane();
    if (! lane.isValid())
    {
        // Cartel de bienvenida con una curva de ejemplo
        auto card = getLocalBounds().toFloat().withSizeKeepingCentre (460.0f, 214.0f);
        Draw::shadow (g, card, 14.0f, 18);
        g.setGradientFill (juce::ColourGradient (Colours::panelLight, 0.0f, card.getY(), Colours::panel, 0.0f, card.getBottom(), false));
        g.fillRoundedRectangle (card, 14.0f);
        g.setColour (Colours::outline.brighter (0.2f));
        g.drawRoundedRectangle (card.reduced (0.5f), 14.0f, 1.0f);

        auto inner = card.reduced (24.0f, 20.0f);
        auto curveArea = inner.removeFromTop (86.0f).reduced (30.0f, 8.0f);
        g.setColour (juce::Colours::black.withAlpha (0.3f));
        g.fillRoundedRectangle (curveArea.expanded (12.0f, 6.0f), 8.0f);
        const juce::Point<float> pts[] = {
            { curveArea.getX(), curveArea.getBottom() },
            { curveArea.getX() + curveArea.getWidth() * 0.35f, curveArea.getBottom() - curveArea.getHeight() * 0.3f },
            { curveArea.getX() + curveArea.getWidth() * 0.6f, curveArea.getY() },
            { curveArea.getRight(), curveArea.getY() + curveArea.getHeight() * 0.45f } };
        juce::Path curve, fill;
        curve.startNewSubPath (pts[0]);
        for (int i = 1; i < 4; ++i) curve.lineTo (pts[i]);
        fill = curve;
        fill.lineTo (curveArea.getBottomRight());
        fill.lineTo (curveArea.getBottomLeft());
        fill.closeSubPath();
        g.setGradientFill (juce::ColourGradient (Colours::accent.withAlpha (0.3f), 0.0f, curveArea.getY(),
                                                 Colours::accent.withAlpha (0.0f), 0.0f, curveArea.getBottom(), false));
        g.fillPath (fill);
        g.setColour (Colours::accent);
        g.strokePath (curve, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        for (auto p : pts)
        {
            g.setColour (Colours::accentLight);
            g.fillEllipse (juce::Rectangle<float> (9.0f, 9.0f).withCentre (p));
            g.setColour (juce::Colours::black.withAlpha (0.6f));
            g.drawEllipse (juce::Rectangle<float> (9.0f, 9.0f).withCentre (p), 1.0f);
        }

        inner.removeFromTop (12.0f);
        g.setColour (Colours::text);
        g.setFont (Fonts::bold (17.0f));
        g.drawText (txt ("Aún no hay automatizaciones"), inner.removeFromTop (26.0f), juce::Justification::centred);
        g.setColour (Colours::textDim);
        g.setFont (Fonts::regular (13.0f));
        g.drawFittedText (txt ("Pulsa \"+ Nueva\" y elige un control (volumen, filtro, reverb...) "
                               "para que se mueva solo a lo largo de la canción."),
                          inner.toNearestInt(), juce::Justification::centredTop, 3, 1.0f);
        return;
    }

    // Etiquetas de valor
    const auto target = AutomationTarget::parse (lane[ids::target].toString());
    auto fmt = [&target] (float norm)
    {
        const float v = AutomationTarget::fromNormalised (target.param, norm);
        switch (target.param)
        {
            case AutomationTarget::Param::cutoff: return juce::String ((int) v) + " Hz";
            case AutomationTarget::Param::pan:    return std::abs (v) < 0.01f ? juce::String ("Centro") : (v < 0 ? juce::String ("Izquierda") : juce::String ("Derecha"));
            case AutomationTarget::Param::eqLow:
            case AutomationTarget::Param::eqHigh: return juce::String (v, 1) + " dB";
            default:                              return juce::String ((int) (v * 100.0f)) + "%";
        }
    };
    g.setColour (Colours::textDim);
    g.drawText (fmt (1.0f), juce::Rectangle<float> (4.0f, valueToY (1.0f) - 14.0f, 120.0f, 12.0f), juce::Justification::centredLeft);
    g.drawText (fmt (0.0f), juce::Rectangle<float> (4.0f, valueToY (0.0f) + 2.0f, 120.0f, 12.0f), juce::Justification::centredLeft);

    // Curva
    juce::Array<juce::ValueTree> points;
    for (auto p : lane)
        points.add (p);
    std::sort (points.begin(), points.end(), [] (const juce::ValueTree& a, const juce::ValueTree& b)
               { return (double) a[ids::time] < (double) b[ids::time]; });

    if (! points.isEmpty())
    {
        juce::Path path, fill;
        const float y0 = valueToY (points[0][ids::value]);
        path.startNewSubPath (0.0f, y0);
        fill.startNewSubPath (0.0f, (float) getHeight());
        fill.lineTo (0.0f, y0);
        for (auto& p : points)
        {
            path.lineTo (timeToX (p[ids::time]), valueToY (p[ids::value]));
            fill.lineTo (timeToX (p[ids::time]), valueToY (p[ids::value]));
        }
        const float yEnd = valueToY (points.getLast()[ids::value]);
        path.lineTo ((float) getWidth(), yEnd);
        fill.lineTo ((float) getWidth(), yEnd);
        fill.lineTo ((float) getWidth(), (float) getHeight());
        fill.closeSubPath();

        g.setColour (Colours::accent.withAlpha (0.12f));
        g.fillPath (fill);
        g.setColour (Colours::accent);
        g.strokePath (path, juce::PathStrokeType (2.0f));

        for (auto& p : points)
        {
            const float x = timeToX (p[ids::time]), y = valueToY (p[ids::value]);
            g.setColour (juce::Colours::white);
            g.fillEllipse (x - 5.0f, y - 5.0f, 10.0f, 10.0f);
            g.setColour (Colours::accent);
            g.drawEllipse (x - 5.0f, y - 5.0f, 10.0f, 10.0f, 2.0f);
        }
    }

    if (ctx.engine.isSongMode())
    {
        g.setColour (Colours::green);
        g.fillRect (timeToX (ctx.engine.getPlayPosition()), 0.0f, 2.0f, (float) getHeight());
    }
}

void AutomationEditor::mouseDown (const juce::MouseEvent& e)
{
    auto lane = view.getLane();
    if (! lane.isValid())
        return;

    ctx.project.beginTransaction();
    auto point = pointAt (e.position);

    if (e.mods.isPopupMenu())
    {
        if (point.isValid() && lane.getNumChildren() > 1)
            lane.removeChild (point, ctx.project.um());
        return;
    }

    if (! point.isValid())
    {
        point = juce::ValueTree (ids::POINT);
        point.setProperty (ids::time, std::round (xToTime (e.position.x)), nullptr);
        point.setProperty (ids::value, yToValue (e.position.y), nullptr);
        lane.appendChild (point, ctx.project.um());
    }
    dragPoint = point;
}

void AutomationEditor::mouseDrag (const juce::MouseEvent& e)
{
    if (! dragPoint.isValid())
        return;
    dragPoint.setProperty (ids::time, std::round (xToTime (e.position.x)), ctx.project.um());
    dragPoint.setProperty (ids::value, yToValue (e.position.y), ctx.project.um());
}

void AutomationEditor::mouseUp (const juce::MouseEvent&)
{
    dragPoint = {};
}

// ============================================================================
//  AutomationView
// ============================================================================
AutomationView::AutomationView (AppContext& c)
    : ctx (c), helpButton (txt (help::automationTitle), txt (help::automationText)), editor (c, *this)
{
    addAndMakeVisible (helpButton);

    addAndMakeVisible (laneBox);
    laneBox.setTooltip (txt ("La automatización que estás editando."));
    laneBox.onChange = [this]
    {
        auto lane = ctx.project.automation().getChild (laneBox.getSelectedId() - 1);
        if (lane.isValid())
            ctx.project.getState().setProperty (ids::selectedLane, lane[ids::uid], nullptr);
    };
    makeNonFocusing (laneBox);

    addAndMakeVisible (newButton);
    newButton.setButtonText (txt ("+ Nueva"));
    newButton.setTooltip (txt ("Crear una automatización nueva: elige qué control quieres mover."));
    newButton.onClick = [this] { showNewLaneMenu(); };
    makeNonFocusing (newButton);

    addAndMakeVisible (deleteButton);
    deleteButton.setButtonText ("Eliminar");
    deleteButton.setTooltip (txt ("Elimina la automatización seleccionada."));
    deleteButton.onClick = [this]
    {
        auto lane = getLane();
        if (lane.isValid())
        {
            ctx.project.beginTransaction();
            ctx.project.automation().removeChild (lane, ctx.project.um());
        }
    };
    makeNonFocusing (deleteButton);

    addAndMakeVisible (infoLabel);
    infoLabel.setColour (juce::Label::textColourId, Colours::textDim);
    infoLabel.setText (txt ("La automatización solo actúa en modo CANCIÓN."), juce::dontSendNotification);

    addAndMakeVisible (editor);
    ctx.project.addChangeListener (this);
    refresh();
    startTimerHz (20);
}

AutomationView::~AutomationView()
{
    ctx.project.removeChangeListener (this);
}

juce::ValueTree AutomationView::getLane()
{
    auto lanes = ctx.project.automation();
    auto lane = lanes.getChildWithProperty (ids::uid, ctx.project.getState()[ids::selectedLane]);
    return lane.isValid() ? lane : lanes.getChild (0);
}

void AutomationView::changeListenerCallback (juce::ChangeBroadcaster*)
{
    refresh();
}

void AutomationView::refresh()
{
    laneBox.clear (juce::dontSendNotification);
    int i = 1;
    for (auto lane : ctx.project.automation())
        laneBox.addItem (lane[ids::name].toString(), i++);
    laneBox.setSelectedId (ctx.project.automation().indexOf (getLane()) + 1, juce::dontSendNotification);
    deleteButton.setEnabled (getLane().isValid());
    editor.repaint();
}

void AutomationView::timerCallback()
{
    if (ctx.engine.isPlaying() && isShowing())
        editor.repaint();
}

void AutomationView::showNewLaneMenu()
{
    using P = AutomationTarget::Param;
    juce::PopupMenu menu;
    std::vector<AutomationTarget> targets;

    auto add = [&] (juce::PopupMenu& m, AutomationTarget::Kind kind, int index, P param)
    {
        AutomationTarget t;
        t.kind = kind;
        t.index = index;
        t.param = param;
        targets.push_back (t);
        m.addItem ((int) targets.size(), AutomationTarget::paramName (param));
    };

    menu.addSectionHeader (txt ("Mezclador"));
    for (int track = 0; track < numMixerTracks; ++track)
    {
        juce::PopupMenu sub;
        for (auto p : { P::volume, P::pan, P::reverbMix, P::delayMix, P::eqLow, P::eqHigh })
            add (sub, AutomationTarget::Kind::mixer, track, p);
        const auto name = track == 0 ? juce::String ("Master")
                                     : "Pista " + juce::String (track) + " (" + ctx.project.getMixerTrack (track)[ids::name].toString() + ")";
        menu.addSubMenu (name, sub);
    }

    menu.addSectionHeader ("Canales");
    for (auto ch : ctx.project.channels())
    {
        juce::PopupMenu sub;
        add (sub, AutomationTarget::Kind::channel, ch[ids::uid], P::volume);
        add (sub, AutomationTarget::Kind::channel, ch[ids::uid], P::pan);
        if (ch[ids::type].toString() == "synth")
            add (sub, AutomationTarget::Kind::channel, ch[ids::uid], P::cutoff);
        menu.addSubMenu (ch[ids::name].toString(), sub);
    }

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&newButton), [this, targets] (int result)
    {
        if (result > 0 && result <= (int) targets.size())
        {
            ctx.project.beginTransaction();
            ctx.project.addAutomationLane (targets[(size_t) result - 1]);
        }
    });
}

void AutomationView::paint (juce::Graphics& g)
{
    drawPanel (g, getLocalBounds(), headerArea, txt ("Automatización"));
}

void AutomationView::resized()
{
    auto area = getLocalBounds();
    headerArea = area.removeFromTop (42);
    auto h = headerArea.reduced (8, 7);
    helpButton.setBounds (h.removeFromLeft (26));
    h.removeFromLeft (150);
    laneBox.setBounds (h.removeFromLeft (300)); h.removeFromLeft (8);
    newButton.setBounds (h.removeFromLeft (90)); h.removeFromLeft (6);
    deleteButton.setBounds (h.removeFromLeft (90)); h.removeFromLeft (12);
    infoLabel.setBounds (h);
    editor.setBounds (area.reduced (8));
}
}
